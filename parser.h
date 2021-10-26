#ifndef __PARSER_H__
#define __PARSER_H__
#include "ast.h"
#include "scanner.h"
class Parser {
  std::vector<Token> tokens;
  int current;

  Token advance();
  Token peek();

  Token consume(TokenType type, std::string error);

  bool match(int count, ...);

  void checkEof();
  bool eof() { return current == tokens.size(); };

  std::vector<Declaration*> program();

  Declaration* decl();
  Statement* stmt();
  PrintStmt* printStmt();
  BlockStmt* blockStmt();
  ExprStmt* exprStmt();
  VarDecl* varDecl();

  Expr* expression();
  Expr* assignment();
  Expr* equality();
  Expr* comparsion();
  Expr* term();
  Expr* factor();
  Expr* unary();
  Expr* primary();

 public:
  std::vector<Declaration*> parse(const std::vector<Token>& tokens);
};
#endif
