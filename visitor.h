#pragma once
#include "context.h"
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
class AssertStmt;
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
  virtual void visit(AssertStmt* st) = 0;
  virtual void visit(PrintStmt* st) = 0;
  virtual void visit(VarDecl* d) = 0;
  virtual void visit(FunDecl* d) = 0;
  virtual void visit(BlockStmt* d) = 0;
  virtual void visit(IfStmt* d) = 0;
  virtual void visit(WhileStmt* d) = 0;
  virtual void visit(BreakStmt* d) = 0;
  virtual void visit(ReturnStmt* d) = 0;
};

class PrintVisitor : public DeclVisitor {
 public:
  // Return a PrintVisitor with an inner block scopping
  virtual void visit(Declaration* d) override;

  virtual void visit(ExprStmt* st) override;
  virtual void visit(AssertStmt* st) override;
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

class PrintExprVisitor : public ExprVisitor {
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
