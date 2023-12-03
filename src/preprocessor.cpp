#include <fstream>
#include <sstream>
#include <regex>

#include "preprocessor.h"

extern std::string import_directory;

std::stringstream preprocessorDestination;
std::ofstream destinationFile;

std::string getFileName(std::string fullName)
{
  int lastBackSlash = fullName.rfind("/") + 1;
  if (lastBackSlash != -1)
  {
    fullName = fullName.substr(lastBackSlash);
  }
  return fullName;
}

void preprocessor(std::string filename) {
  std::ifstream program(filename);
  std::string line;
  std::regex import_("\\?ИМПОРТ\\(\".+\"\\)\\.\n*\r*");
  if (program.is_open() == false)
    throw std::runtime_error(("Cannot open file: " + getFileName(filename)).c_str());
  while (std::getline(program, line)) {
    if (std::regex_match(line, import_)) {
      int pos1 = line.find("\"") + 1;
      int pos2 = line.find("\"", pos1);
      std::string importFileName = line.substr(pos1, pos2 - pos1);
      if (importFileName.find("..\\") != -1 || importFileName.find("../") != -1)
        throw std::runtime_error("An attempt was made to access the parent directory with \"../\".");
      preprocessor(import_directory + importFileName);
    } else {
      preprocessorDestination << line << "\n";
    }
  }
  program.close();
}

std::stringstream& preprocessor_run(std::string filename) {
  preprocessor(filename);
  return preprocessorDestination;
}

void preprocessor_interactive(std::istream *source) {
  std::string line;
  std::regex import_("\\?ИМПОРТ\\(\".+\"\\)\\.\n*\r*");
  while (std::getline(*source, line)) {
    if (line.empty())
      break;
    if (std::regex_match(line, import_)) {
      int pos1 = line.find("\"") + 1;
      int pos2 = line.find("\"", pos1);
      std::string importFileName = line.substr(pos1, pos2 - pos1);
      if (importFileName.find("..\\") != -1 || importFileName.find("../") != -1)
        throw std::runtime_error("An attempt was made to access the parent directory with \"../\".");
      preprocessor(import_directory + importFileName);
    } else {
      preprocessorDestination << line << "\n";
    }
  }
}

std::stringstream& preprocessor_run_interactive(std::istream *source) {
  preprocessor_interactive(source);
  return preprocessorDestination;
}

std::list<std::string> preprocessor_run_on_source(std::list<std::string> source) {
  std::string destinationName = "./prologd_temporary_file_1.pld";
  destinationFile.open(destinationName);
  for (std::string& s : source) {
    destinationFile << s << '\n';
  }
  destinationFile.close();
  std::stringstream& in = preprocessor_run("./prologd_temporary_file_1.pld");
  std::string line;
  source.clear();
  while (std::getline(in, line)) {
      source.push_back(line);
  }
  destinationFile.close();
  return source;
}
