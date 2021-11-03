#pragma once
#include <string>
class CmdArgs {
  bool debug_;
  bool printIR_;
  bool printLex_;
  bool compile_;
  bool link_;
  std::string fileName;

 public:
  CmdArgs(int argc, char** argv);
  bool debug() { return debug_; };
  bool printIR() { return printIR_; };
  bool printLex() { return printLex_; };
  bool compile() { return compile_; };
  bool link() { return link_; };
  std::string getFileName() { return fileName; };
};
extern CmdArgs* options;
