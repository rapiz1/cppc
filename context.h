#pragma once

#include <map>
#include <memory>
#include <string>

#include "llvm.h"
#include "type.h"

struct Record {
  std::string id;
  Type type;
  llvm::Argument* arg;
};

class Literal;
enum class ReturnReason { NORMAL, RETURN, BREAK, CONTINUE };
struct ReturnResult {
  ReturnReason reason;
  llvm::Value* value;
};
class Scope {
  Scope* parent;
  typedef std::map<std::string, Record> VarRec;
  std::shared_ptr<VarRec> varRec;
  ReturnResult* reason;

  bool localCount(std::string);
  void setOrCreateVar(std::string, Record r);

 public:
  Scope(Scope* parent = nullptr, ReturnResult* reason = nullptr)
      : parent(parent), reason(reason) {
    varRec = std::make_shared<VarRec>();
  }

  bool count(std::string);

  void define(std::string, Record r);
  void set(std::string, Record r);
  Record get(std::string);

  void setReason(ReturnResult r);
  ReturnResult getReason();

  bool isWrapped() const { return parent; }

  Scope wrap() { return Scope(this, reason); }
  Scope wrapWithReason(ReturnResult* reason) { return Scope(this, reason); };
};
