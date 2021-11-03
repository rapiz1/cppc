#pragma once

#include <map>
#include <memory>
#include <string>

#include "llvm.h"
#include "type.h"

class FunDecl;
struct Record {
  std::string id;
  Type type;
  llvm::AllocaInst* addr;
};

class Literal;
struct Trace {
  llvm::Function *llvmFun;
  FunDecl* fun;
  llvm::BasicBlock* endB;
};
class Scope {
  Scope* parent;
  typedef std::map<std::string, Record> VarRec;
  std::shared_ptr<VarRec> varRec;
  Trace* trace;

  bool localCount(std::string);
  void setOrCreateVar(std::string, Record r);

 public:
  Scope(Scope* parent = nullptr, Trace* trace = nullptr)
      : parent(parent), trace(trace) {
    varRec = std::make_shared<VarRec>();
  }

  bool count(std::string);

  void define(std::string, Record r);
  void set(std::string, Record r);
  Record get(std::string);

  void setTrace(Trace r);
  Trace getTrace();

  bool isWrapped() const { return parent; }

  Scope wrap() { return Scope(this, trace); }
  Scope wrapWithTrace(Trace* t) { return Scope(this, t); };
};
