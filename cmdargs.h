#pragma once
#include <string>
enum RunMode { REPL, FROM_FILE };
class CmdArgs {
  bool debug, allowRedefine;
  RunMode mode;
  std::string fileName;

 public:
  CmdArgs(int argc, char** argv);
  bool getDebug() { return debug; }
  bool getAllowRedefine() { return allowRedefine; }
  RunMode getMode() { return mode; }
  std::string getFileName() { return fileName; }
};
extern CmdArgs* options;
