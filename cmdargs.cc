#include "cmdargs.h"

#include <cstdlib>
#include <iostream>
#include <string>

const std::string usage = "Usage: clox [script]\n";

CmdArgs::CmdArgs(int argc, char** argv) {
  debug = false;
  if (argc > 2) {
    std::cerr << usage;
    exit(-1);
  } else if (argc == 2) {
    fileName = std::string(argv[1]);
    mode = RunMode::FROM_FILE;
  } else {
    mode = RunMode::REPL;
  }
}
