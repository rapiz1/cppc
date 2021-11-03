#pragma once
#include <string>
class CmdArgs {
  bool debug;
  bool irOutput;
  bool lexOutput;
  std::string fileName;

 public:
  CmdArgs(int argc, char** argv);
  bool getDebug() { return debug; };
  bool printIR() { return irOutput; };
  bool printLex() { return lexOutput; };
  std::string getFileName() { return fileName; };
};
extern CmdArgs* options;
