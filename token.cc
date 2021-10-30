#include "token.h"

#include <map>

#include "token_convert.h"
std::ostream& operator<<(std::ostream& out, Token token) {
  out << "line: " << token.line << ", " << token.lexeme;
  if (token.tokenType == TokenType::TEOF) out << " EOF";
  return out;
}

TokenType string2keyword(const std::string& s) {
  auto it = str2token.find(s);
  if (it == str2token.end())
    return INVALID;
  else
    return it->second;
}
