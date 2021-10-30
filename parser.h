#ifndef __PARSER_H__
#define __PARSER_H__
#include "ast.h"
#include "scanner.h"
class Parser {
  std::vector<Token> tokens;
  size_t current;

  Token advance();
  Token peek();

  Token consume(TokenType type, std::string error);

  bool match(int count, ...);

  void checkEof();
  bool eof() { return current == tokens.size(); };

  std::vector<Declaration*> program();

  Declaration* decl();       // STMT | VAR_DECL
  Statement* stmt();         // PRINT_STMT | BLOCK_STMT | EXPR_STMT | IF_STMT |
                             // FOR_STMT | WHILE_STMT
  PrintStmt* printStmt();    // PRINT EXPRESSION ;
  BlockStmt* blockStmt();    // '{' PROGRAM '}';
  ExprStmt* exprStmt();      // EXPRESSION ;
  IfStmt* ifStmt();          // IF EXPRESSION STMT (ELSE STMT)?
  BlockStmt* forStmt();      // FOR '(' VAR_DECL EXPRESSION; EXPRESSION ')'
  WhileStmt* whileStmt();    // WHILE '(' EXPRESSION ')' STMT
  ReturnStmt* returnStmt();  // RETURN EXPR;
  VarDecl* varDecl();        // VAR IDENTIFIER (EQUAL EXPRESSION)? ;
  FunDecl* funDecl();        // FUN IDENTIFIER '(' ARGS? ')' BLOCK
  Args args();               // ID (, ID)*
  RealArgs real_args();      // EXPR (, EXPR)*

  Expr* expression();
  Expr* assignment();
  Expr* equality();
  Expr* comparsion();
  Expr* term();
  Expr* factor();
  Expr* unary();  // UNARY = CALL | (! | -) CALL
  Expr* call();   // CALL = PRIM ('(' ARGS? ')')*
  Expr* primary();

 public:
  std::vector<Declaration*> parse(const std::vector<Token>& tokens);
};
#endif
