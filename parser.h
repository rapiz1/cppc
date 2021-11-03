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
                             // FOR_STMT | WHILE_STMT | ASSERT_STMT
  AssertStmt* assertStmt();  // ASSERT EXPR ;
  PrintStmt* printStmt();    // PRINT EXPRESSION ;
  BlockStmt* blockStmt();    // '{' PROGRAM '}';
  ExprStmt* exprStmt();      // EXPRESSION ;
  IfStmt* ifStmt();          // IF EXPRESSION STMT (ELSE STMT)?
  BlockStmt* forStmt();      // FOR '(' VAR_DECL EXPRESSION; EXPRESSION ')'
  WhileStmt* whileStmt();    // WHILE '(' EXPRESSION ')' STMT
  ReturnStmt* returnStmt();  // RETURN EXPR;
  TypedVar typedVar();       // (VAR | INT | DOUBLE | CHAR) ID
  VarDecl* varDecl(Type type, Token id);  // TYPE ('['SIZE']')? IDENTIFIER
                                          // (EQUAL EXPRESSION)? ;
  FunDecl* funDecl(Type type,
                   Token id);  // TYPEDVAR '(' ARGS? ')' BLOCK?
  Args args();                 // TYPEDVAR (, TYPEDVAR)*
  RealArgs real_args();        // EXPR (, EXPR)*

  Expr* expression();  // ASSIGN
  Expr* assignment();  // LVAL '=' EQUALITY | EQUALITY
  Expr* equality();    // COMP ('==' | '!=') COMP | COMP
  Expr* comparsion();  // TERM ('>' | '>=' | '<' | '<=') TERM | TERM
  Expr* term();        // FACTOR (('+' | '-') FACTOR)*
  Expr* factor();      // UNARY (('/' | '*') UNARY)*
  Expr* unary();       // (! | - | -- | ++)* POSTFIX
  Expr* postfix();     // CALL (++ | --)*
  Expr* call();        // INDEX ('(' ARGS? ')')*
  Expr* index();       // PRIM ('[' NUMBER ']')?
  Expr* primary();

 public:
  std::vector<Declaration*> parse(const std::vector<Token>& tokens);
};
#endif
