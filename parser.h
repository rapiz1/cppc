#ifndef __PARSER_H__
#define __PARSER_H__
#include "expr.h"
#include "scanner.h"
class Parser {
  std::vector<Token> tokens;
  int current;

  Token advance();
  Token peek();

  Token consume(TokenType type, std::string error);

  bool match(int count, ...);

  bool eof() { return current == tokens.size(); };

  std::vector<Stmt*> program();

  Stmt* stmt();
  PrintStmt* printStmt();
  VarDecl* varDecl();

  Expr* expression();
  Expr* equality();
  Expr* comparsion();
  Expr* term();
  Expr* factor();
  Expr* unary();
  Expr* primary();

 public:
  std::vector<Stmt*> parse(const std::vector<Token>& tokens);
};
#endif
