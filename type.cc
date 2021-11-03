#include "type.h"

#include <sstream>

Type::operator std::string() {
  std::string ret = "variable type";
  std::stringstream ss;
  ss << arraySize;
  if (isArray) ret += " , array of " + ss.str();
  return ret;
}
