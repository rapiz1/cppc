#include "parser.h"

#include <cassert>
#include <cstdarg>
#include <iostream>
#include <sstream>
#include <stack>

#include "log.h"

void Parser::checkEof() {
  if (eof()) {
    std::cerr << "Unexpected eof\n";
    exit(-1);
  }
}

Token Parser::advance() {
  checkEof();
  return tokens[current++];
}

Token Parser::peek() {
  checkEof();
  return tokens[current];
}

Token Parser::consume(TokenType type, std::string error) {
  Token t = advance();
  if (t.tokenType != type) {
    std::cerr << "line " << t.line << ": " << error << std::endl;
    std::cerr << "\t but got char `" << t.lexeme << "`\n";
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

std::vector<Declaration*> Parser::program() {
  std::vector<Declaration*> prog;
  while (!eof() && !match(1, RIGHT_BRACE)) {
    prog.push_back(decl());
  }
  return prog;
}

Declaration* Parser::decl() {
  Declaration* d = nullptr;
  TypedVar var;
  switch (peek().tokenType) {
    case VAR:
    case INT:
    case DOUBLE:
    case FLOAT:
    case CHAR:
    case BOOL:
    case VOID:
      var = typedVar();
      if (match(1, LEFT_PAREN))
        d = funDecl(var.type, var.id);
      else
        d = varDecl(var.type, var.id);
      break;
    default:
      d = stmt();
      break;
  }
  assert(d);
  return d;
}

Statement* Parser::stmt() {
  Statement* s = nullptr;
  switch (peek().tokenType) {
    case PRINT:
      s = printStmt();
      break;
    case LEFT_BRACE:
      s = blockStmt();
      break;
    case FOR:
      s = forStmt();
      break;
    case WHILE:
      s = whileStmt();
      break;
    case IF:
      s = ifStmt();
      break;
    case BREAK:
      s = new BreakStmt();
      advance();
      consume(SEMICOLON, "Expect `;` after break");
      break;
    case CONTINUE:
      s = new ContStmt();
      advance();
      consume(SEMICOLON, "Expect `;` after continue");
      break;
    case RETURN:
      s = returnStmt();
      break;
    case ASSERT:
      s = assertStmt();
      break;
    default:
      s = exprStmt();
      break;
  }

  assert(s);
  return s;
}

BlockStmt* Parser::blockStmt() {
  BlockStmt* b;
  consume(LEFT_BRACE, "Expect `{` at the begining of a block");
  b = new BlockStmt(program());
  consume(RIGHT_BRACE, "Expect `}` at the end of a block");
  return b;
}

ReturnStmt* Parser::returnStmt() {
  ReturnStmt* s = nullptr;
  Expr* e = nullptr;
  consume(RETURN, "Expect `return`");
  if (!match(1, SEMICOLON)) {
    e = expression();
  }
  consume(SEMICOLON, "Expect `;` after return");
  s = new ReturnStmt(e);
  return s;
}

IfStmt* Parser::ifStmt() {
  IfStmt* i = nullptr;
  consume(IF, "Expect `if`");

  Expr* e = expression();
  assert(e);

  Statement* tb = stmt();
  assert(tb);

  Statement* fb = nullptr;
  if (match(1, ELSE)) {
    advance();
    fb = stmt();
  }

  i = new IfStmt(e, tb, fb);
  assert(i);
  return i;
}

WhileStmt* Parser::whileStmt() {
  WhileStmt* w = nullptr;
  consume(WHILE, "Expect `while`");

  Expr* e = expression();
  assert(e);

  Statement* b = stmt();
  assert(b);

  w = new WhileStmt(e, b);

  assert(w);
  return w;
}

// Desugar `for` to `while`
BlockStmt* Parser::forStmt() {
  BlockStmt* f = nullptr;
  consume(FOR, "Expect `for`");

  consume(LEFT_PAREN, "Expect `(`");

  Declaration* init = decl();
  assert(init);

  Expr* condition = expression();
  assert(condition);

  consume(SEMICOLON, "Expect `;` after for condition");

  Expr* inc = expression();
  assert(inc);

  consume(RIGHT_PAREN, "Expect `)`");

  Statement* b = stmt();
  assert(b);

  /*
  { // outer
    init;
    while (condition), with update exprStmt inc
      b;
  }
  */

  Program outer;

  outer.push_back(init);
  outer.push_back(new WhileStmt(condition, b, new ExprStmt(inc)));

  f = new BlockStmt(outer);
  return f;
}

Statement* Parser::printStmt() {
  abortMsg("print statement is deprecated");
  return nullptr;
}

Statement* Parser::assertStmt() {
  abortMsg("assert statement is deprecated");
  return nullptr;
}

ExprStmt* Parser::exprStmt() {
  ExprStmt* s = new ExprStmt(expression());

  consume(SEMICOLON, "Expect `;` at the end of a expr statement");
  assert(s);
  return s;
}

TypedVar Parser::typedVar() {
  Type::Base base;
  Token t = advance();
  switch (t.tokenType) {
    case INT:
      base = Type::Base::INT;
      break;
    case DOUBLE:
    case FLOAT:
      base = Type::Base::DOUBLE;
      break;
    case CHAR:
      base = Type::Base::CHAR;
      break;
    case BOOL:
      base = Type::Base::BOOL;
      break;
    case VOID:
      base = Type::Base::VOID;
      break;
    default:
      std::cerr << "Unexpected type " << peek().lexeme << std::endl;
      exit(-1);
  }
  Token id = consume(IDENTIFIER, "Expect an identifer for variable");
  Type type = {base};

  while (match(1, LEFT_SQUARE)) {
    // parse array type
    advance();
    auto num = consume(NUMBER, "Expect a number literal for array size");

    int dim = 0;
    std::stringstream ss(num.lexeme);
    ss >> dim;
    if (dim <= 0) {
      std::cerr << "Invalid size of array at line " << num.line << std::endl;
      exit(-1);
    }

    type.dims.push_back(dim);

    type.isArray = true;
    consume(RIGHT_SQUARE, "Expect `]` affter array size");
  }

  if (type.isArray) {
    type.arraySize = 1;
    for (auto x : type.dims) type.arraySize *= x;
  }

  return {type, id};
}

VarDecl* Parser::varDecl(Type type, Token id) {
  Expr* init = nullptr;
  if (match(1, EQUAL)) {
    advance();
    init = expression();
  }

  VarDecl* s = new VarDecl(type, id.lexeme, init);
  consume(SEMICOLON, "Expect a `;` at the end of a declaration");

  assert(s);
  return s;
}

Args Parser::args() {
  Args args;

  args.push_back(typedVar());
  while (match(1, COMMA)) {
    advance();
    args.push_back(typedVar());
  }

  return args;
}
RealArgs Parser::real_args() {
  // FIXME:
  RealArgs args;

  args.push_back(expression());
  while (match(1, COMMA)) {
    advance();
    args.push_back(expression());
  }

  return args;
}

FunDecl* Parser::funDecl(Type retType, Token id) {
  consume(LEFT_PAREN, "Expect `(` as argument list begins");

  Args a;
  if (!match(1, RIGHT_PAREN)) {
    a = args();
  }

  consume(RIGHT_PAREN, "Expect `)` as argument list ends");

  BlockStmt* b = nullptr;
  if (match(1, LEFT_BRACE))
    b = blockStmt();
  else
    consume(SEMICOLON, "Expect `,` after function prototype");

  return new FunDecl(id.lexeme, a, b, retType);
}

Expr* Parser::expression() { return assignment(); }

Expr* Parser::assignment() {
  Expr* e = equality();
  if (match(6, EQUAL, SLASH_EQUAL, STAR_EQUAL, PLUS_EQUAL, MINUS_EQUAL,
            PERCENT_EQUAL)) {
    Token op = advance();
    Expr* v = assignment();
    if (!e->isLval()) {
      std::cerr << e << " is not a left value. At line " << op.line
                << std::endl;
      exit(-1);
    }

    if (op.tokenType != EQUAL) {  // desuger composition assignment
      switch (op.tokenType) {
        case STAR_EQUAL:
          v = new Binary(e, Token(STAR, "sugar", -1), v);
          break;
        case SLASH_EQUAL:
          v = new Binary(e, Token(SLASH, "sugar", -1), v);
          break;
        case PLUS_EQUAL:
          v = new Binary(e, Token(PLUS, "sugar", -1), v);
          break;
        case MINUS_EQUAL:
          v = new Binary(e, Token(MINUS, "sugar", -1), v);
          break;
        case PERCENT_EQUAL:
          v = new Binary(e, Token(PERCENT, "sugar", -1), v);
          break;
        default:
          std::cerr << op.lexeme << ". At line  " << op.line << std::endl;
          exit(-1);
      }
    }
    op.tokenType = EQUAL;
    e = new Binary(e, op, v);
  }
  return e;
}

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
  while (match(3, STAR, SLASH, PERCENT)) {
    Token op = advance();
    Expr* right = unary();
    left = new Binary(left, op, right);
  }
  return left;
}

Expr* Parser::unary() {
  std::stack<Token> st;
  while (match(4, MINUS, BANG, PLUSPLUS, MINUSMINUS)) {
    Token op = advance();
    st.push(op);
  }
  Expr* e = postfix();
  while (!st.empty()) {
    auto op = st.top();
    st.pop();
    e = new Unary(op, e);
  }
  return e;
}

Expr* Parser::postfix() {
  Expr* p = call();
  while (match(2, PLUSPLUS, MINUSMINUS)) {
    Token op = advance();
    p = new Postfix(op, p);
  }
  return p;
}

Expr* Parser::call() {
  Expr* e = index();
  while (match(1, LEFT_PAREN)) {
    advance();
    RealArgs a;
    if (!match(1, RIGHT_PAREN)) {
      a = real_args();
    }
    e = new Call(e, a);
    consume(RIGHT_PAREN, "Expect `)` after the arugment list");
  }
  return e;
}

Expr* Parser::index() {
  Expr* e = primary();
  std::vector<Expr*> idxs;
  while (match(1, LEFT_SQUARE)) {
    advance();
    idxs.push_back(expression());
    consume(RIGHT_SQUARE, "Expect `]` after indexing");
  }
  if (!idxs.empty()) e = new Index(e, idxs);
  return e;
}

Expr* Parser::primary() {
  Expr* prim = nullptr;
  switch (peek().tokenType) {
    case NUMBER:
      if (peek().lexeme.find('.') != std::string::npos) {
        prim = new Double(advance());
      } else
        prim = new Integer(advance());
      break;
    case STRING:
      prim = new String(advance());
      break;
    case CHARACTER:
      prim = new Char(advance());
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
      consume(RIGHT_PAREN, "Expect `)`");
      break;
    case IDENTIFIER:
      prim = new Variable(advance());
      break;
    default:
      std::cerr << "line " << peek().line << ": Unexpected lexeme "
                << peek().lexeme << std::endl;
      exit(-1);
      break;
  }
  assert(prim != nullptr);
  return prim;
}

std::vector<Declaration*> Parser::parse(const std::vector<Token>& tokens) {
  this->tokens = tokens;
  current = 0;
  return program();
}
