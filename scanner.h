#ifndef __CLOX_SCANNER__
#define __CLOX_SCANNER__
#include <string>
#include <vector>

#include "token.h"
class Scanner {
  const std::string &source;
  size_t start, current, line;  // current is the char we're about to consume

  bool badbit;
  std::string errorMessage;
  std::vector<Token> tokens;

  std::string get_lexeme(TokenType type);

  char advance();
  char peek();
  char peekNext();
  bool match(char expected);

  void error(std::string message);

  inline bool eof() { return current == source.size(); }

  void string();
  void number();
  void identifierOrKeyword();

  void addToken(TokenType type);
  void scanToken();

 public:
  Scanner(const std::string &source) : source(source) {
    start = current = line = badbit = 0;
  }

  inline bool good() { return !badbit; }
  inline std::string getError() { return errorMessage; }

  std::vector<Token> scanTokens();
};
#endif
