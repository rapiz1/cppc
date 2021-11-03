#include "cmdargs.h"

#include <cstdlib>
#include <iostream>
#include <string>

const std::string usage = "Usage: clox [source]\n";

CmdArgs::CmdArgs(int argc, char** argv) {
  compile_ = true;
  link_ = true;

  printLex_ = false;
  printIR_ = false;

  debug_ = true;
  if (debug_) {
    printLex_ = false;
    printIR_ = true;
  }

  if (argc == 2) {
    fileName = std::string(argv[1]);
  } else {
    std::cerr << usage;
    exit(-1);
  }
}
