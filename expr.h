#ifndef __EXPR_H__
#define __EXPR_H__
#include <string>

#include "token.h"

class ExprVisitor;

class Expr {
 public:
  virtual operator std::string() = 0;
  virtual void accept(ExprVisitor* v) = 0;
};

class Binary;
class Unary;
class Literal;
class Number;
class String;

class ExprVisitor {
 public:
  virtual void visit(Expr* expr) = 0;
  virtual void visit(Literal* expr) = 0;
  virtual void visit(Binary* expr) = 0;
  virtual void visit(Unary* expr) = 0;
};

class PrintVisitor : public ExprVisitor {
 public:
  void visit(Expr* expr);
  void visit(Literal* expr);
  void visit(Binary* expr);
  void visit(Unary* expr);
};

class EvalVisitor : public ExprVisitor {
  Expr* value = nullptr;

 public:
  void visit(Expr* expr);
  void visit(Literal* expr);
  void visit(Binary* expr);
  void visit(Unary* expr);
  Expr* getValue() { return value; }
};

class Binary : public Expr {
 protected:
  Expr* left;
  Expr* right;
  Token op;

 public:
  Binary(Expr* left, Token op, Expr* right)
      : left(left), op(op), right(right){};
  operator std::string();

  void accept(ExprVisitor* v) { v->visit(this); }
  friend class EvalVisitor;
};

class Unary : public Expr {
 protected:
  Token op;
  Expr* child;

 public:
  Unary(Token op, Expr* child) : op(op), child(child){};
  operator std::string();
  void accept(ExprVisitor* v) { v->visit(this); }
  friend class EvalVisitor;
};

class Literal : public Expr {
 public:
  virtual bool isTruthy() = 0;
};

class Number : public Literal {
 protected:
  double value;

 public:
  Number(Token token);
  Number(double value) : value(value){};
  operator std::string();

  bool isTruthy();

  void accept(ExprVisitor* v) { v->visit(this); }
  friend class EvalVisitor;
};

class String : public Literal {
 protected:
  std::string value;

 public:
  String(Token token) { value = token.lexeme; }
  String(std::string value) : value(value){};
  operator std::string();

  bool isTruthy();

  void accept(ExprVisitor* v) { v->visit(this); }
  friend class EvalVisitor;
};

class Boolean : public Literal {
 protected:
  bool value;

 public:
  Boolean(bool value) : value(value){};
  Boolean(Token token) { value = token.tokenType == TRUE; }
  operator std::string();

  bool isTruthy() { return value; }

  void accept(ExprVisitor* v) { v->visit(this); }
  friend class EvalVisitor;
};
#endif
