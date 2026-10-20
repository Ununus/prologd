#include <fstream>
#include <filesystem>
#include <sstream>
#include <regex>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "path_utf8.hpp"
#include "preprocessor.h"

std::stringstream preprocessorDestination;

namespace {

//: Каталог запущенного exe и каталоги из командной строки: их заполняет
//: prologdc.cpp, а спрашивает поиск файла `?ИМПОРТ`.
std::string executableDirectory;
std::vector<std::string> importDirectories;

const std::regex &importLine() {
  static const std::regex line("(?:\\?|:-)[ ]*ИМПОРТ\\(\".+\"\\)\\.\n*\r*");
  return line;
}

//: Строка `:- ИМЕНА(N).` — указание, как читать имена, из Пролога-Д 2.x.
const std::regex &namesLine() {
  static const std::regex line(":-[ ]*ИМЕНА.*\n*\r*");
  return line;
}

std::string fileName(const std::filesystem::path &path) {
  return prologd_path::to_utf8(path.filename());
}

//: Имя, названное в строке `?ИМПОРТ("имя")`.
std::string importName(const std::string &line) {
  const size_t pos1 = line.find('"') + 1;
  const size_t pos2 = line.find('"', pos1);
  return line.substr(pos1, pos2 - pos1);
}

void checkImportName(const std::string &name) {
  if (name.find("..\\") != std::string::npos || name.find("../") != std::string::npos) {
    throw std::runtime_error("An attempt was made to access the parent directory with \"../\".");
  }
}

bool isFile(const std::filesystem::path &path) {
  std::error_code error;
  return std::filesystem::is_regular_file(path, error);
}

std::filesystem::path findImport(const std::string &name, const std::filesystem::path &fromDirectory) {
  const std::filesystem::path import = prologd_path::from_utf8(name);
  if (import.is_absolute()) {
    return isFile(import) ? import : std::filesystem::path();
  }
  std::vector<std::filesystem::path> bases;
  if (!fromDirectory.empty()) {
    bases.push_back(fromDirectory);
  }
  for (const std::string &directory : importDirectories) {
    bases.push_back(prologd_path::from_utf8(directory));
  }
  if (!executableDirectory.empty()) {
    const std::filesystem::path executable = prologd_path::from_utf8(executableDirectory);
    bases.push_back(executable);
    bases.push_back(executable / "stdlib");
  }
  std::error_code error;
  const std::filesystem::path current = std::filesystem::current_path(error);
  if (!error) {
    bases.push_back(current);
  }
  for (const std::filesystem::path &base : bases) {
    const std::filesystem::path candidate = base / import;
    if (isFile(candidate)) {
      return candidate;
    }
  }
  return std::filesystem::path();
}

void preprocessFile(const std::filesystem::path &filename, const std::filesystem::path &baseDirectory) {
  std::ifstream program(filename);
  if (program.is_open() == false) {
    throw std::runtime_error(("Cannot open file: " + fileName(filename)).c_str());
  }
  const std::filesystem::path base = baseDirectory.empty() ? filename.parent_path() : baseDirectory;
  std::string line;
  while (std::getline(program, line)) {
    std::replace(line.begin(), line.end(), '\t', ' ');  // the interpreter doesn't like tabs
    if (std::regex_match(line, namesLine())) {
      continue;  // `:- ИМЕНА(N).` — движок читает имена по-старому и её не знает
    }
    if (!std::regex_match(line, importLine())) {
      preprocessorDestination << line << "\n";
      continue;
    }
    const std::string name = importName(line);
    checkImportName(name);
    const std::filesystem::path found = findImport(name, base);
    if (found.empty()) {
      throw std::runtime_error("Cannot open file: " + name);
    }
    preprocessFile(found, std::filesystem::path());
  }
  program.close();
}

void preprocessInteractive(std::istream *source) {
  std::string line;
  while (std::getline(*source, line)) {
    if (line.empty()) {
      break;
    }
    if (std::regex_match(line, namesLine())) {
      continue;  // `:- ИМЕНА(N).` — движок читает имена по-старому и её не знает
    }
    if (!std::regex_match(line, importLine())) {
      preprocessorDestination << line << "\n";
      continue;
    }
    const std::string name = importName(line);
    checkImportName(name);
    const std::filesystem::path found = findImport(name, std::filesystem::path());
    if (found.empty()) {
      throw std::runtime_error("Cannot open file: " + name);
    }
    preprocessFile(found, std::filesystem::path());
  }
}

}  // namespace

void preprocessor_set_executable_directory(std::string directory) {
  executableDirectory = std::move(directory);
}

void preprocessor_set_import_directories(std::vector<std::string> directories) {
  importDirectories = std::move(directories);
}

std::string preprocessor_find_import(std::string name, std::string from_directory) {
  const std::filesystem::path found = findImport(name, prologd_path::from_utf8(from_directory));
  return found.empty() ? std::string() : prologd_path::to_utf8(found);
}

std::stringstream& preprocessor_run(std::string filename) {
  preprocessorDestination = std::stringstream();
  preprocessFile(prologd_path::from_utf8(filename), std::filesystem::path());
  return preprocessorDestination;
}

std::stringstream& preprocessor_run_interactive(std::istream *source) {
  preprocessorDestination = std::stringstream();
  preprocessInteractive(source);
  return preprocessorDestination;
}

std::list<std::string> preprocessor_run_on_source(std::list<std::string> source, std::string source_path) {
  // Свой результат, как и у разбора файла: вызывающий мог уже прочитать прошлый
  // до конца, и без этого новый текст приклеился бы к старому.
  preprocessorDestination = std::stringstream();
  std::filesystem::path tmpPath = std::filesystem::temp_directory_path();
  tmpPath /= "prologd_temporary_file_1.pld";

  // Поток локальный: разбор может бросить ошибку `?ИМПОРТ`, и тогда файл
  // должен быть закрыт — иначе следующая программа читала бы его пустым.
  {
    std::ofstream destinationFile(tmpPath);
    if (!destinationFile.is_open()) {
      throw std::runtime_error(("Cannot open file: " + prologd_path::to_utf8(tmpPath)).c_str());
    }
    for (std::string& s : source) {
      destinationFile << s << '\n';
    }
  }
  preprocessFile(tmpPath, prologd_path::from_utf8(source_path).parent_path());
  std::stringstream& in = preprocessorDestination;
  std::string line;
  source.clear();
  while (std::getline(in, line)) {
      source.push_back(line);
  }
  preprocessorDestination = std::stringstream();
  return source;
}
