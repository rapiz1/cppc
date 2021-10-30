#pragma once

#include <map>
#include <memory>
#include <string>

class Literal;
enum class ReturnReason { NORMAL, RETURN, BREAK, CONTINUE };
struct ReturnResult {
  ReturnReason reason;
  Literal* value;
};
class ExecContext {
  ExecContext* parent;
  typedef std::map<std::string, Literal*> VarRec;
  std::shared_ptr<VarRec> varRec;
  ReturnResult* reason;

  bool localCount(std::string);
  void setOrCreateVar(std::string, Literal* expr);

 public:
  ExecContext(ExecContext* parent = nullptr, ReturnResult* reason = nullptr)
      : parent(parent), reason(reason) {
    varRec = std::make_shared<VarRec>();
  }

  bool count(std::string);

  void define(std::string, Literal* expr);
  void set(std::string, Literal* expr);
  Literal* get(std::string);

  void setReason(ReturnResult r);
  ReturnResult getReason();

  bool isWrapped() const { return parent; }

  ExecContext wrap() { return ExecContext(this, reason); }
  ExecContext wrapWithReason(ReturnResult* reason) {
    return ExecContext(this, reason);
  };
};
