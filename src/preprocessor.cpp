#include "preprocessor.h"

extern std::string import_directory;
std::ofstream destinationFile;

void preprocessor(std::string filename) {
  std::ifstream program(filename);
  std::string line;
  std::regex import("\\?ÈÌÏÎÐÒ\\(\".+\"\\)\\.");
  if (program.is_open() == false)
    throw std::exception(("Cannot open file: " + filename).c_str());
  while (program >> line) {
    if (std::regex_match(line, import)) {
      int pos1 = line.find("\"") + 1;
      int pos2 = line.find("\"", pos1);
      std::string importFileName = line.substr(pos1, pos2 - pos1);
      if (importFileName.find("..\\") != -1 || importFileName.find("../") != -1)
        throw std::exception("An attempt was made to access the parent directory with \"..\\\".");
      preprocessor(import_directory + importFileName);
    } else {
      destinationFile << line << "\n";
    }
  }
  program.close();
}

std::string preprocessor_run(std::string filename) {
  int lastBackSlash = filename.rfind("\\") + 1;
  std::string destinationName = filename.substr(0, lastBackSlash) + "prologd_temporary_file.pld";
  destinationFile.open(destinationName);
  preprocessor(filename);
  destinationFile.close();
  return destinationName;
}