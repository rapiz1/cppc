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
class Index;
class Double;
class Integer;
class Boolean;
class Char;
class String;

class Declaration;
class VarDecl;
class FunDecl;

class Statement;
class ExprStmt;
class BlockStmt;
class IfStmt;
class ForStmt;
class WhileStmt;
class BreakStmt;
class ReturnStmt;

typedef std::vector<Declaration*> Program;

class AstVisitor {
 public:
  virtual void visit(Expr* expr) = 0;
  virtual void visit(Literal* expr) = 0;
  virtual void visit(Integer* expr) = 0;
  virtual void visit(Double* expr) = 0;
  virtual void visit(Boolean* expr) = 0;
  virtual void visit(Char* expr) = 0;
  virtual void visit(String* expr) = 0;
  virtual void visit(Binary* expr) = 0;
  virtual void visit(Unary* expr) = 0;
  virtual void visit(Postfix* expr) = 0;
  virtual void visit(Variable* expr) = 0;
  virtual void visit(Call* expr) = 0;
  virtual void visit(Index* expr) = 0;

  virtual void visit(Declaration* d) = 0;
  virtual void visit(ExprStmt* st) = 0;
  virtual void visit(VarDecl* d) = 0;
  virtual void visit(FunDecl* d) = 0;
  virtual void visit(BlockStmt* d) = 0;
  virtual void visit(IfStmt* d) = 0;
  virtual void visit(WhileStmt* d) = 0;
  virtual void visit(BreakStmt* d) = 0;
  virtual void visit(ReturnStmt* d) = 0;
};

class CodeGenVisitor : public AstVisitor {
  Scope scope;
  llvmWrapper l;
  llvm::Value* value = nullptr;
  llvm::AllocaInst* addr = nullptr;
  Type type = {};  // only used for array. other type information is passed by
                   // llvm::Value*
  bool terminate = false;

 public:
  CodeGenVisitor(Scope scope, llvmWrapper l) : scope(scope), l(l) {
    value = nullptr;
    addr = nullptr;
    type = {};
    terminate = false;
  };

  CodeGenVisitor wrap();
  CodeGenVisitor wrapWithTrace(Trace* r);

  auto getMod() { return l.mod; }
  auto getCtx() { return l.ctx; }
  void visit(Declaration* d) override;

  void visit(ExprStmt* st) override;
  void visit(VarDecl* d) override;
  void visit(FunDecl* d) override;
  void visit(BlockStmt* d) override;
  void visit(IfStmt* d) override;
  void visit(WhileStmt* d) override;
  void visit(BreakStmt* d) override;
  void visit(ReturnStmt* d) override;

  void visit(Expr* expr) override;
  void visit(Literal* expr) override;
  void visit(Integer* expr) override;
  void visit(Double* expr) override;
  void visit(Boolean* expr) override;
  void visit(Char* expr) override;
  void visit(String* expr) override;
  void visit(Binary* expr) override;
  void visit(Unary* expr) override;
  void visit(Postfix* expr) override;
  void visit(Variable* expr) override;
  void visit(Call* expr) override;
  void visit(Index* expr) override;

  Type getType() { return type; }
  void setType(Type t) { type = t; }
  void setValue(llvm::Value* v) { value = v; }
  void setAddr(llvm::AllocaInst* a) { addr = a; }
  void setTuple(llvm::Value* v, llvm::AllocaInst* a = nullptr) {
    value = v;
    addr = a;
  }

  llvm::Value* getValue() { return value; }
  llvm::AllocaInst* getAddr() { return addr; }
};

class GraphGenVisitor : public AstVisitor {
  std::string content;
  int nodeNum;
  int rootNode;

  std::string getTagName(int id) const;

  int addNode(std::string desc);
  void addTo(int x, int y);

 public:
  void output() const;
  void visitProgram(const Program& prog);
  void visit(Declaration* d) override;

  void visit(ExprStmt* st) override;
  void visit(VarDecl* d) override;
  void visit(FunDecl* d) override;
  void visit(BlockStmt* d) override;
  void visit(IfStmt* d) override;
  void visit(WhileStmt* d) override;
  void visit(BreakStmt* d) override;
  void visit(ReturnStmt* d) override;

  void visit(Expr* expr) override;
  void visit(Literal* expr) override;
  void visit(Integer* expr) override;
  void visit(Double* expr) override;
  void visit(Boolean* expr) override;
  void visit(Char* expr) override;
  void visit(String* expr) override;
  void visit(Binary* expr) override;
  void visit(Unary* expr) override;
  void visit(Postfix* expr) override;
  void visit(Variable* expr) override;
  void visit(Call* expr) override;
  void visit(Index* expr) override;
};
