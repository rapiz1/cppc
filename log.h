#pragma once
#include <iostream>
#include <string>

inline void abortMsg(std::string s) {
  std::cerr << s << std::endl;
  exit(-1);
};

inline void unimplemented() {
  std::cerr << "unimplemented\n";
  exit(-1);
}
