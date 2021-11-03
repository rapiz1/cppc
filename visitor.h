#pragma once
#include "context.h"
#include "llvm.h"

class Expr;
class Literal;
class Binary;
class Unary;
class Postfix;
class Variable;
class Call;
class Double;
class Integer;
class Boolean;
class Char;

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

class ExprVisitor {
 public:
  virtual void visit(Expr* expr) = 0;
  virtual void visit(Literal* expr) = 0;
  virtual void visit(Integer* expr) = 0;
  virtual void visit(Double* expr) = 0;
  virtual void visit(Boolean* expr) = 0;
  virtual void visit(Char* expr) = 0;
  virtual void visit(Binary* expr) = 0;
  virtual void visit(Unary* expr) = 0;
  virtual void visit(Postfix* expr) = 0;
  virtual void visit(Variable* expr) = 0;
  virtual void visit(Call* expr) = 0;
};

class CodeGenExprVisitor : public ExprVisitor {
  Scope scope;
  llvmWrapper l;
  llvm::Value* value = nullptr;
  llvm::AllocaInst* addr = nullptr;

 public:
  CodeGenExprVisitor(Scope scope, llvmWrapper l) : scope(scope), l(l){};
  void visit(Expr* expr) override;
  void visit(Literal* expr) override;
  void visit(Integer* expr) override;
  void visit(Double* expr) override;
  void visit(Boolean* expr) override;
  void visit(Char* expr) override;
  void visit(Binary* expr) override;
  void visit(Unary* expr) override;
  void visit(Postfix* expr) override;
  void visit(Variable* expr) override;
  void visit(Call* expr) override;

  void setValue(llvm::Value* v) { value = v; }
  void setAddr(llvm::AllocaInst* a) { addr = a; }
  void setTuple(llvm::Value* v, llvm::AllocaInst* a = nullptr) {
    value = v;
    addr = a;
  }

  llvm::Value* getValue() { return value; }
  llvm::AllocaInst* getAddr() { return addr; }
};

class CodeGenVisitor : public DeclVisitor {
  Scope scope;
  llvmWrapper l;
  bool terminate;

 public:
  CodeGenVisitor(Scope scope, llvmWrapper l)
      : scope(scope), l(l), terminate(false){};
  CodeGenVisitor wrap();
  CodeGenVisitor wrapWithTrace(Trace* r);
  auto getMod() { return l.mod; }
  auto getCtx() { return l.ctx; }
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
