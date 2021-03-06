#ifndef __EXPR_H__
#define __EXPR_H__
#include <string>
#include <vector>

#include "token.h"
#include "type.h"
#include "visitor.h"

class AstNode {
 public:
  virtual operator std::string() = 0;
  virtual void accept(AstVisitor* v) = 0;
};

class Expr : public AstNode {
 public:
  virtual bool isLval() const = 0;
};

class Declaration : public AstNode {};

class Statement : public Declaration {};

class ExprStmt : public Statement {
  Expr* expr;

 public:
  ExprStmt(Expr* expr) : expr(expr){};
  operator std::string() override { return std::string(*expr); };
  Expr* getExpr() const { return expr; }

  void accept(AstVisitor* v) override { v->visit(this); }
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
  Program getProgram() const { return decls; }

  void accept(AstVisitor* v) override { v->visit(this); }
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

  Expr* getCondition() const { return condition; };
  Statement* getTrue() const { return true_branch; };
  Statement* getFalse() const { return false_branch; };
  operator std::string() override { return "ifstmt"; };

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

class WhileStmt : public Statement {
 protected:
  Expr* condition;
  Statement* body;
  Statement* update;

 public:
  WhileStmt(Expr* condition, Statement* body, Statement* update = nullptr)
      : condition(condition), body(body), update(update){};
  Expr* getCondition() const { return condition; };
  Statement* getBody() const { return body; };
  Statement* getUpdate() const { return update; };
  operator std::string() override { return "whilestmt"; };

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

class BreakStmt : public Statement {
 public:
  operator std::string() override { return "break"; };

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

class ContStmt : public Statement {
 public:
  operator std::string() override { return "continue"; };

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

class ReturnStmt : public Statement {
  Expr* expr;

 public:
  ReturnStmt(Expr* expr) : expr(expr){};
  Expr* getExpr() const { return expr; };

  operator std::string() override { return "return" + std::string(*expr); };

  void accept(AstVisitor* v) override { v->visit(this); }
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
  Type getType() const { return type; };
  std::string name() const { return identifier; };
  Expr* getInit() const { return init; };
  operator std::string() override {
    return "var " + identifier + " = " + std::string(*init);
  };

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

typedef std::vector<TypedVar> Args;

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

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class PrintVisitor;
  friend class CodeGenVisitor;
};

typedef std::vector<Expr*> RealArgs;
class Call : public Expr {
  Expr* callee;
  RealArgs args;

 public:
  Call(Expr* callee, RealArgs args) : callee(callee), args(args){};
  Expr* getCallee() const { return callee; }
  RealArgs getArgs() const { return args; };
  operator std::string() override { return "call " + std::string(*callee); };
  bool isLval() const override { return false; }

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class CodeGenVisitor;
};

class Index : public Expr {
  Expr* base;
  std::vector<Expr*> idxs;

 public:
  Index(Expr* base, std::vector<Expr*> idxs) : base(base), idxs(idxs){};
  Expr* getBase() const { return base; };
  std::vector<Expr*> getIdxs() const { return idxs; };
  operator std::string() override { return "index " + std::string(*base); };
  bool isLval() const override { return true; }

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class CodeGenVisitor;
};

class Binary : public Expr {
 protected:
  Expr* left;
  Token op;
  Expr* right;

 public:
  Binary(Expr* left, Token op, Expr* right)
      : left(left), op(op), right(right){};
  Token getOp() const { return op; };
  Expr* getLeft() const { return left; };
  Expr* getRight() const { return right; };
  operator std::string() override;
  bool isLval() const override { return false; }

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class CodeGenVisitor;
};

class Unary : public Expr {
 protected:
  Token op;
  Expr* child;

 public:
  Unary(Token op, Expr* child) : op(op), child(child){};
  Token getOp() const { return op; };
  Expr* getChild() const { return child; };
  operator std::string() override;
  bool isLval() const override { return false; }

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class CodeGenVisitor;
};

class Postfix : public Expr {
 protected:
  Token op;
  Expr* child;

 public:
  Postfix(Token op, Expr* child) : op(op), child(child){};
  Token getOp() const { return op; };
  Expr* getChild() const { return child; };
  operator std::string() override { return "postfix " + op.lexeme; };
  bool isLval() const override { return false; }

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class CodeGenVisitor;
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
  int getValue() const { return value; };
  operator std::string() override;

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class CodeGenVisitor;
};

class Double : public Literal {
 protected:
  double value;

 public:
  Double(Token token);
  Double(double value) : value(value){};
  double getValue() const { return value; }
  operator std::string() override;

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class CodeGenVisitor;
};

class String : public Literal {
 protected:
  std::string value;

 public:
  String(Token token) { value = token.lexeme; }
  String(std::string value) : value(value){};
  std::string getValue() const { return value; };
  operator std::string() override;

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class CodeGenVisitor;
};

class Char : public Literal {
 protected:
  char value;

 public:
  Char(Token token);
  Char(char value) : value(value){};
  char getValue() const { return value; };
  operator std::string() override;

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class CodeGenVisitor;
};

class Boolean : public Literal {
 protected:
  bool value;

 public:
  Boolean(bool value) : value(value){};
  Boolean(Token token) { value = token.tokenType == TRUE; }
  bool getValue() const { return value; }
  operator std::string() override;

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class CodeGenVisitor;
};

class Function : public Literal {
 protected:
  FunDecl* fun;

 public:
  Function(FunDecl* fun) : fun(fun){};
  FunDecl* getFun() const { return fun; };
  operator std::string() override { return std::string(*fun); };

  void accept(AstVisitor* v) override { v->visit(this); }
  friend class CodeGenVisitor;
};

class Variable : public Expr {
 protected:
  std::string name;

 public:
  Variable(std::string name) : name(name){};
  Variable(Token token) { name = token.lexeme; };
  bool isLval() const override { return true; }
  std::string getName() const { return name; };

  operator std::string() override { return name; };

  void accept(AstVisitor* v) override { v->visit(this); };
  friend class CodeGenVisitor;
};
#endif
