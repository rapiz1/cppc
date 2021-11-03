#include "cmdargs.h"

#include <cstdlib>
#include <iostream>
#include <string>

const std::string usage = "Usage: clox [source]\n";

CmdArgs::CmdArgs(int argc, char** argv) {
  lexOutput = false;
  irOutput = false;

  debug = true;
  if (debug) {
    lexOutput = false;
    irOutput = true;
  }

  if (argc == 2) {
    fileName = std::string(argv[1]);
  } else {
    std::cerr << usage;
    exit(-1);
  }
}
