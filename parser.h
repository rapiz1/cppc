#ifndef __PARSER_H__
#define __PARSER_H__
#include "expr.h"
#include "scanner.h"
class Parser {
  std::vector<Token> tokens;
  int current;

  Token advance();
  Token peek();

  bool match(int count, ...);

  Expr* expression();
  Expr* equality();
  Expr* comparsion();
  Expr* term();
  Expr* factor();
  Expr* unary();
  Expr* primary();

 public:
  Expr* parse(const std::vector<Token>& tokens);
};
#endif
