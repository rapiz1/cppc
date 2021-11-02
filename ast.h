#ifndef __EXPR_H__
#define __EXPR_H__
#include <string>
#include <vector>

#include "token.h"
#include "type.h"
#include "visitor.h"

class Converter;
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

  void accept(DeclVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

class PrintStmt : public Statement {
 protected:
  Expr* expr;

 public:
  PrintStmt(Expr* expr) : expr(expr){};
  operator std::string() override { return "print " + std::string(*expr); };

  void accept(DeclVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

class AssertStmt : public Statement {
 protected:
  Expr* expr;

 public:
  AssertStmt(Expr* expr) : expr(expr){};
  operator std::string() override { return "assert " + std::string(*expr); };

  void accept(DeclVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

typedef std::vector<Declaration*> Program;
class BlockStmt : public Statement {
 protected:
  Program decls;

 public:
  BlockStmt(Program decls) : decls(decls){};
  operator std::string() override {
    std::string content;
    for (auto d : decls) content += std::string(*d);
    return "{ " + content + " }";
  }

  void accept(DeclVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

class IfStmt : public Statement {
 protected:
  Expr* condition;
  Statement *true_branch, *false_branch;

 public:
  IfStmt(Expr* condition, Statement* true_branch, Statement* false_branch)
      : condition(condition),
        true_branch(true_branch),
        false_branch(false_branch){};

  operator std::string() override { return "ifstmt"; };

  void accept(DeclVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

class WhileStmt : public Statement {
 protected:
  Expr* condition;
  Statement* body;

 public:
  WhileStmt(Expr* condition, Statement* body)
      : condition(condition), body(body){};
  operator std::string() override { return "whilestmt"; };

  void accept(DeclVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

class BreakStmt : public Statement {
 public:
  operator std::string() override { return "break"; };

  void accept(DeclVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

class ReturnStmt : public Statement {
  Expr* expr;

 public:
  ReturnStmt(Expr* expr) : expr(expr){};

  operator std::string() override { return "return" + std::string(*expr); };

  void accept(DeclVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

class VarDecl : public Declaration {
 protected:
  Type type;
  std::string identifier;
  Expr* init;

 public:
  VarDecl(Type type, std::string id, Expr* init)
      : type(type), identifier(id), init(init){};
  operator std::string() override {
    return "var " + identifier + " = " + std::string(*init);
  };

  void accept(DeclVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

struct FormalArg {
  Type type;
  Token token;
};

typedef std::vector<FormalArg> Args;

class FunDecl : public Declaration {
 protected:
  std::string identifier;
  Args args;
  BlockStmt* body;
  Type retType;

 public:
  FunDecl(std::string id, Args args, BlockStmt* body, Type retType)
      : identifier(id), args(args), body(body), retType(retType){};
  operator std::string() override { return "function " + identifier; };

  std::string name() const { return identifier; };
  Args getArgs() const { return args; };
  BlockStmt* getBody() const { return body; };

  void accept(DeclVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

typedef std::vector<Expr*> RealArgs;
class Call : public Expr {
  Expr* callee;
  RealArgs args;

 public:
  Call(Expr* callee, RealArgs args) : callee(callee), args(args){};
  operator std::string() override { return "call " + std::string(*callee); };
  bool isLval() const override { return false; }

  void accept(ExprVisitor* v) override { v->visit(this); }
  friend class CodeGenExprVisitor;
};

class Binary : public Expr {
 protected:
  Expr* left;
  Token op;
  Expr* right;

 public:
  Binary(Expr* left, Token op, Expr* right)
      : left(left), op(op), right(right){};
  operator std::string() override;
  bool isLval() const override { return false; }

  void accept(ExprVisitor* v) override { v->visit(this); }
  friend class CodeGenExprVisitor;
};

class Unary : public Expr {
 protected:
  Token op;
  Expr* child;

 public:
  Unary(Token op, Expr* child) : op(op), child(child){};
  operator std::string() override;
  bool isLval() const override { return false; }

  void accept(ExprVisitor* v) override { v->visit(this); }
  friend class CodeGenExprVisitor;
};

class Postfix : public Expr {
 protected:
  Token op;
  Expr* child;

 public:
  Postfix(Token op, Expr* child) : op(op), child(child){};
  operator std::string() override { return "postfix " + op.lexeme; };
  bool isLval() const override { return false; }

  void accept(ExprVisitor* v) override { v->visit(this); }
  friend class CodeGenExprVisitor;
};

class Literal : public Expr {
 public:
  bool isLval() const override { return false; }
};

class Integer : public Literal {
 protected:
  int value;

 public:
  Integer(Token token);
  Integer(int value) : value(value){};
  operator std::string() override;

  void accept(ExprVisitor* v) override { v->visit(this); }
  friend class CodeGenExprVisitor;

  friend class Converter;
};

class Double : public Literal {
 protected:
  double value;

 public:
  Double(Token token);
  Double(double value) : value(value){};
  operator std::string() override;

  void accept(ExprVisitor* v) override { v->visit(this); }
  friend class CodeGenExprVisitor;

  friend class Converter;
};

class String : public Literal {
 protected:
  std::string value;

 public:
  String(Token token) { value = token.lexeme; }
  String(std::string value) : value(value){};
  operator std::string() override;

  void accept(ExprVisitor* v) override { v->visit(this); }
  friend class CodeGenExprVisitor;

  friend class Converter;
};

class Boolean : public Literal {
 protected:
  bool value;

 public:
  Boolean(bool value) : value(value){};
  Boolean(Token token) { value = token.tokenType == TRUE; }
  operator std::string() override;

  void accept(ExprVisitor* v) override { v->visit(this); }
  friend class CodeGenExprVisitor;

  friend class Converter;
};

class Function : public Literal {
 protected:
  FunDecl* fun;

 public:
  Function(FunDecl* fun) : fun(fun){};
  operator std::string() override { return std::string(*fun); };

  void accept(ExprVisitor* v) override { v->visit(this); }
  friend class CodeGenExprVisitor;

  friend class Converter;
};

class Variable : public Expr {
 protected:
  std::string name;

 public:
  Variable(std::string name) : name(name){};
  Variable(Token token) { name = token.lexeme; };
  bool isLval() const override { return true; }

  operator std::string() override { return name; };

  void accept(ExprVisitor* v) override { v->visit(this); };
  friend class CodeGenExprVisitor;
};
#endif
