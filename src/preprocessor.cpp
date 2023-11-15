#include <fstream>
#include <regex>

#include "preprocessor.h"

extern std::string import_directory;

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
  std::regex import_("\\?ИМПОРТ\\(\".+\"\\)\\.");
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
      destinationFile << line << "\n";
    }
  }
  program.close();
}

std::string preprocessor_run(std::string filename) {
  int lastBackSlash = filename.rfind("/") + 1;
  std::string destinationName = filename.substr(0, lastBackSlash) + "prologd_temporary_file.pld";
  destinationFile.open(destinationName);
  preprocessor(filename);
  destinationFile.close();
  return destinationName;
}

std::list<std::string> preprocessor_run_on_source(std::list<std::string> source) {
  std::string destinationName = "./prologd_temporary_file_1.pld";
  destinationFile.open(destinationName);
  for (std::string& s : source) {
    destinationFile << s << '\n';
  }
  destinationFile.close();
  std::string dest = preprocessor_run("./prologd_temporary_file_1.pld");
  std::ifstream in(dest);
  std::string line;
  source.clear();
  while (std::getline(in, line)) {
      source.push_back(line);
  }
  destinationFile.close();
  return source;
}
