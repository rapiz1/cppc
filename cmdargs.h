#pragma once
#include <string>
enum RunMode { REPL, FROM_FILE };
class CmdArgs {
  bool debug;
  RunMode mode;
  std::string fileName;

 public:
  CmdArgs(int argc, char** argv);
  bool getDebug() { return debug; }
  RunMode getMode() { return mode; }
  std::string getFileName() { return fileName; }
};
