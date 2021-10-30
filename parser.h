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

  Declaration* decl();     // STMT | VAR_DECL
  Statement* stmt();       // PRINT_STMT | BLOCK_STMT | EXPR_STMT | IF_STMT |
                           // FOR_STMT | WHILE_STMT
  PrintStmt* printStmt();  // PRINT EXPRESSION ;
  BlockStmt* blockStmt();  // { PROGRAM };
  ExprStmt* exprStmt();    // EXPRESSION ;
  IfStmt* ifStmt();        // IF EXPRESSION BLOCK
  ForStmt* forStmt();      // FOR (VAR_DECL EXPRESSION; EXPRESSION)
  WhileStmt* whileStmt();  // WHILE (EXPRESSION) BLOCK
  VarDecl* varDecl();      // VAR IDENTIFIER [= EXPRESSION] ;

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
