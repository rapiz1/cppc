#ifndef __EXPR_H__
#define __EXPR_H__
#include <string>
#include <vector>

#include "context.h"
#include "token.h"

class Expr;
class Literal;
class Binary;
class Unary;
class Postfix;
class Variable;
class Call;

class Declaration;
class VarDecl;
class FunDecl;

class Statement;
class PrintStmt;
class ExprStmt;
class BlockStmt;
class IfStmt;
class ForStmt;
class WhileStmt;
class BreakStmt;
class ReturnStmt;

class DeclVisitor {
 public:
  virtual void visit(Declaration* d) = 0;

  virtual void visit(ExprStmt* st) = 0;
  virtual void visit(PrintStmt* st) = 0;
  virtual void visit(VarDecl* d) = 0;
  virtual void visit(FunDecl* d) = 0;
  virtual void visit(BlockStmt* d) = 0;
  virtual void visit(IfStmt* d) = 0;
  virtual void visit(WhileStmt* d) = 0;
  virtual void visit(BreakStmt* d) = 0;
  virtual void visit(ReturnStmt* d) = 0;
};

class ExecVisitor : public DeclVisitor {
  ExecContext context;

 public:
  ExecVisitor(ExecContext context) : context(context){};
  // Return a ExecVisitor with an inner block scopping
  ExecVisitor wrap();
  ExecVisitor wrapWithReason();
  virtual void visit(Declaration* d) override;

  virtual void visit(ExprStmt* st) override;
  virtual void visit(PrintStmt* st) override;
  virtual void visit(VarDecl* d) override;
  virtual void visit(FunDecl* d) override;
  virtual void visit(BlockStmt* d) override;
  virtual void visit(IfStmt* d) override;
  virtual void visit(WhileStmt* d) override;
  virtual void visit(BreakStmt* d) override;
  virtual void visit(ReturnStmt* d) override;
};

class ExprVisitor {
 public:
  virtual void visit(Expr* expr) = 0;
  virtual void visit(Literal* expr) = 0;
  virtual void visit(Binary* expr) = 0;
  virtual void visit(Unary* expr) = 0;
  virtual void visit(Postfix* expr) = 0;
  virtual void visit(Variable* expr) = 0;
  virtual void visit(Call* expr) = 0;
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
  Literal* value = nullptr;

 public:
  EvalVisitor(ExecContext context) : context(context){};
  void visit(Expr* expr);
  void visit(Literal* expr);
  void visit(Binary* expr);
  void visit(Unary* expr);
  void visit(Postfix* expr);
  void visit(Variable* expr);
  void visit(Call* expr);
  Literal* getValue() { return value; }
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

  void accept(DeclVisitor* v) { v->visit(this); }
  friend class ExecVisitor;
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

  void accept(DeclVisitor* v) { v->visit(this); }
  friend class ExecVisitor;
};

class WhileStmt : public Statement {
 protected:
  Expr* condition;
  Statement* body;

 public:
  WhileStmt(Expr* condition, Statement* body)
      : condition(condition), body(body){};
  operator std::string() override { return "whilestmt"; };

  void accept(DeclVisitor* v) { v->visit(this); }
  friend class ExecVisitor;
};

class BreakStmt : public Statement {
 public:
  operator std::string() override { return "break"; };

  void accept(DeclVisitor* v) { v->visit(this); }
  friend class ExecVisitor;
};

class ReturnStmt : public Statement {
  Expr* expr;

 public:
  ReturnStmt(Expr* expr) : expr(expr){};

  operator std::string() override { return "return"; };

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

typedef std::vector<Token> Args;
class FunDecl : public Declaration {
 protected:
  std::string identifier;
  Args args;
  BlockStmt* body;

 public:
  FunDecl(std::string id, Args args, BlockStmt* body)
      : identifier(id), args(args), body(body){};
  operator std::string() override { return "function " + identifier; };

  std::string name() const { return identifier; };
  Args getArgs() const { return args; };
  BlockStmt* getBody() const { return body; };

  void accept(DeclVisitor* v) { v->visit(this); }
  friend class ExecVisitor;
};

typedef std::vector<Expr*> RealArgs;
class Call : public Expr {
  Expr* callee;
  RealArgs args;

 public:
  Call(Expr* callee, RealArgs args) : callee(callee), args(args){};
  operator std::string() { return "callable"; };
  bool isLval() const override { return false; }

  void accept(ExprVisitor* v) { v->visit(this); }
  friend class EvalVisitor;
};

class Binary : public Expr {
 protected:
  Expr* left;
  Token op;
  Expr* right;

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

class Postfix : public Expr {
 protected:
  Token op;
  Expr* child;

 public:
  Postfix(Token op, Expr* child) : op(op), child(child){};
  operator std::string() { return "postfix " + op.lexeme; };
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

class Function : public Literal {
 protected:
  FunDecl* fun;

 public:
  Function(FunDecl* fun) : fun(fun){};
  operator std::string() { return std::string(*fun); };

  bool isTruthy() { return fun != nullptr; }

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
