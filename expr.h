#ifndef __EXPR_H__
#define __EXPR_H__
#include <string>

#include "context.h"
#include "token.h"

class Expr;
class Literal;
class Binary;
class Unary;
class Variable;

class Statement;
class Declaration;
class PrintStmt;
class ExprStmt;
class VarDecl;

class DeclVisitor {
 public:
  virtual void visit(Declaration* d) = 0;
  virtual void visit(ExprStmt* st) = 0;
  virtual void visit(PrintStmt* st) = 0;
  virtual void visit(VarDecl* d) = 0;
};

class ExecVisitor : public DeclVisitor {
  ExecContext context;

 public:
  ExecVisitor(ExecContext context) : context(context){};
  virtual void visit(Declaration* d) override;
  virtual void visit(ExprStmt* st) override;
  virtual void visit(PrintStmt* st) override;
  virtual void visit(VarDecl* d) override;
};

class ExprVisitor {
 public:
  virtual void visit(Expr* expr) = 0;
  virtual void visit(Literal* expr) = 0;
  virtual void visit(Binary* expr) = 0;
  virtual void visit(Unary* expr) = 0;
  virtual void visit(Variable* expr) = 0;
};

class PrintVisitor : public ExprVisitor {
 public:
  void visit(Expr* expr);
  void visit(Literal* expr);
  void visit(Binary* expr);
  void visit(Unary* expr);
  void visit(Variable* expr);
};

class EvalVisitor : public ExprVisitor {
  ExecContext context;
  Expr* value = nullptr;

 public:
  EvalVisitor(ExecContext context) : context(context){};
  void visit(Expr* expr);
  void visit(Literal* expr);
  void visit(Binary* expr);
  void visit(Unary* expr);
  void visit(Variable* expr);
  Expr* getValue() { return value; }
};

class Expr {
 public:
  virtual operator std::string() = 0;
  virtual void accept(ExprVisitor* v) = 0;
  virtual bool isLval() const = 0;
};

class Declaration {
 public:
  virtual operator std::string() = 0;
  virtual void accept(DeclVisitor* v) = 0;
};

class Statement : public Declaration {};

class ExprStmt : public Statement {
  Expr* expr;

 public:
  ExprStmt(Expr* expr) : expr(expr){};
  operator std::string() override { return std::string(*expr); };

  void accept(DeclVisitor* v) { v->visit(this); }
  friend class ExecVisitor;
};

class PrintStmt : public Statement {
 protected:
  Expr* expr;

 public:
  PrintStmt(Expr* expr) : expr(expr){};
  operator std::string() override { return "print " + std::string(*expr); };

  void accept(DeclVisitor* v) { v->visit(this); }
  friend class ExecVisitor;
};

class VarDecl : public Declaration {
 protected:
  std::string identifier;
  Expr* init;

 public:
  VarDecl(std::string id, Expr* init) : identifier(id), init(init){};
  operator std::string() override {
    return "var " + identifier + " = " + std::string(*init);
  };

  void accept(DeclVisitor* v) { v->visit(this); }
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
  bool isLval() const override { return false; }

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
  bool isLval() const override { return false; }

  void accept(ExprVisitor* v) { v->visit(this); }
  friend class EvalVisitor;
};

class Literal : public Expr {
 public:
  virtual bool isTruthy() = 0;
  bool isLval() const override { return false; }
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
class Variable : public Expr {
 protected:
  std::string name;

 public:
  Variable(std::string name) : name(name){};
  Variable(Token token) { name = token.lexeme; };
  bool isLval() const override { return true; }

  operator std::string() { return name; };

  void accept(ExprVisitor* v) { v->visit(this); };
  friend class EvalVisitor;
};
#endif
