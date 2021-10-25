#ifndef __EXPR_H__
#define __EXPR_H__
#include <string>

#include "token.h"

class StmtVisitor;
class ExprVisitor;

class AstNode {
 public:
  virtual operator std::string() = 0;
};

class Stmt : public AstNode {
 public:
  virtual void accept(StmtVisitor* v) = 0;
};

class Expr : public AstNode {
 public:
  virtual void accept(ExprVisitor* v) = 0;
};

class Binary;
class Unary;
class Literal;
class Number;
class String;

class PrintStmt;
class VarDecl;

class StmtVisitor {
 public:
  virtual void visit(Stmt* st) = 0;
  virtual void visit(PrintStmt* st) = 0;
  virtual void visit(VarDecl* st) = 0;
};

class ExecVisitor : public StmtVisitor {
 public:
  void visit(Stmt* st) override;
  void visit(PrintStmt* st) override;
  void visit(VarDecl* st) override;
};

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

class PrintStmt : public Stmt {
 protected:
  Expr* expr;

 public:
  PrintStmt(Expr* expr) : expr(expr){};
  operator std::string() override { return "print " + std::string(*expr); };

  void accept(StmtVisitor* v) { v->visit(this); }
  friend class ExecVisitor;
};

class VarDecl : public Stmt {
 protected:
  std::string identifier;
  Expr* init;

 public:
  VarDecl(std::string id, Expr* init) : identifier(id), init(init){};
  operator std::string() override {
    return "var " + identifier + " = " + std::string(*init);
  };

  void accept(StmtVisitor* v) { v->visit(this); }
  friend class ExecVisitor;
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
