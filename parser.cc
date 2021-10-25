#include "parser.h"

#include <cassert>
#include <cstdarg>
#include <iostream>

Token Parser::advance() { return tokens[current++]; }
Token Parser::peek() { return tokens[current]; }
Token Parser::consume(TokenType type, std::string error) {
  Token t = advance();
  if (t.tokenType != type) {
    std::cerr << error << std::endl;
    exit(-1);
  }
  return t;
}

bool Parser::match(int count, ...) {
  if (current == tokens.size()) return false;

  bool ret = false;

  va_list args;
  va_start(args, count);

  for (int i = 0; i < count; i++) {
    TokenType t = (TokenType)va_arg(args, int);
    if (t == peek().tokenType) {
      ret = true;
      break;
    }
  }

  va_end(args);

  return ret;
}

std::vector<Stmt*> Parser::program() {
  std::vector<Stmt*> prog;
  while (!eof()) {
    prog.push_back(stmt());
  }
  return prog;
}

Stmt* Parser::stmt() {
  Stmt* s = nullptr;
  switch (peek().tokenType) {
    case PRINT:
      s = printStmt();
      break;
    case VAR:
      s = varDecl();
      break;
  }
  consume(SEMICOLON, "Expect `;` at the end of a statement");

  assert(s);
  return s;
}

PrintStmt* Parser::printStmt() {
  consume(PRINT, "Expect keyword `print`");
  PrintStmt* s = new PrintStmt(expression());

  assert(s);
  return s;
}

VarDecl* Parser::varDecl() {
  consume(VAR, "Expect a `var` declaration");

  Token id = consume(IDENTIFIER, "Expect an identifier");

  VarDecl* s = new VarDecl(id.lexeme, expression());

  assert(s);
  return s;
}

Expr* Parser::expression() { return equality(); }

Expr* Parser::equality() {
  Expr* left = comparsion();
  while (match(2, BANG_EQUAL, EQUAL_EQUAL)) {
    Token op = advance();
    Expr* right = comparsion();
    left = new Binary(left, op, right);
  };
  return left;
}

Expr* Parser::comparsion() {
  Expr* left = term();
  while (match(4, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL)) {
    Token op = advance();
    Expr* right = term();
    left = new Binary(left, op, right);
  }
  return left;
}

Expr* Parser::term() {
  Expr* left = factor();
  while (match(2, PLUS, MINUS)) {
    Token op = advance();
    Expr* right = factor();
    left = new Binary(left, op, right);
  }
  return left;
}

Expr* Parser::factor() {
  Expr* left = unary();
  while (match(2, STAR, SLASH)) {
    Token op = advance();
    Expr* right = unary();
    left = new Binary(left, op, right);
  }
  return left;
}

Expr* Parser::unary() {
  if (match(2, MINUS, BANG)) {
    Token op = advance();
    return new Unary(op, primary());
  } else {
    return primary();
  }
}

Expr* Parser::primary() {
  Expr* prim = nullptr;
  switch (peek().tokenType) {
    case NUMBER:
      prim = new Number(advance());
      break;
    case STRING:
      prim = new String(advance());
      break;
    case TRUE:
    case FALSE:
      prim = new Boolean(advance());
      break;
    case NIL:
      // FIXME:
      assert(false);
      break;
    case LEFT_PAREN:
      advance();
      prim = expression();
      assert(advance().tokenType == RIGHT_PAREN);
      break;
  }
  assert(prim != nullptr);
  return prim;
}

std::vector<Stmt*> Parser::parse(const std::vector<Token>& tokens) {
  this->tokens = tokens;
  current = 0;
  return program();
}
