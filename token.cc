#include "token.h"

#include <map>

#include "token_convert.h"
std::ostream& operator<<(std::ostream& out, Token token) {
  out << "line: " << token.line << ", " << token.lexeme;
  if (token.tokenType == TokenType::TEOF) out << " EOF";
  return out;
}

TokenType string2identifier(const std::string& s) {
  const static std::map<std::string, TokenType> str2token = {
      {"and", AND},   {"class", CLASS}, {"else", ELSE},     {"false", FALSE},
      {"fun", FUN},   {"for", FOR},     {"if", IF},         {"nil", NIL},
      {"or", OR},     {"print", PRINT}, {"return", RETURN}, {"super", SUPER},
      {"this", THIS}, {"true", TRUE},   {"var", VAR},       {"while", WHILE},
  };
  auto it = str2token.find(s);
  if (it == str2token.end())
    return INVALID;
  else
    return it->second;
}
