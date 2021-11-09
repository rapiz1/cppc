#include "ast.h"

#include <string.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>

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

String::operator std::string() { return value; }

Char::operator std::string() {
  string s;
  s.push_back(value);
  return s;
}

Integer::Integer(Token token) {
  std::stringstream ss(token.lexeme);
  ss >> value;
}

Double::Double(Token token) {
  std::stringstream ss(token.lexeme);
  ss >> value;
}

Boolean::operator std::string() { return value ? "true" : "false"; }

Char::Char(Token token) {
  std::string s = token.lexeme;
  if (s.size() == 1)
    value = s[0];
  else
    abortMsg("bad char literal");
}
