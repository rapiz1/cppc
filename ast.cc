#include "ast.h"

#include <string.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>

#include "exception.h"
#include "log.h"
using std::string;

Binary::operator std::string() {
  return "(" + string(*left) + op.lexeme + string(*right) + ")";
}

Unary::operator std::string() { return op.lexeme + string(*child); }

Integer::operator std::string() {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

Double::operator std::string() {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

String::operator std::string() { return "\"" + value + "\""; }

Integer::Integer(Token token) {
  std::stringstream ss(token.lexeme);
  ss >> value;
}

Double::Double(Token token) {
  std::stringstream ss(token.lexeme);
  ss >> value;
}

Boolean::operator std::string() { return value ? "true" : "false"; }
