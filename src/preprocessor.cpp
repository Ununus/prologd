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
  std::regex import("\\?ИМПОРТ\\(\".+\"\\)\\.");
  if (program.is_open() == false)
    throw std::runtime_error(("Cannot open file: " + getFileName(filename)).c_str());
  while (program >> line) {
    if (std::regex_match(line, import)) {
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