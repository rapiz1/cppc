#pragma once

#include <map>
#include <memory>
#include <string>

class Literal;
enum class ReturnReason { NORMAL, RETURN, BREAK, CONTINUE };
class ExecContext {
  ExecContext* parent;
  typedef std::map<std::string, Literal*> VarRec;
  std::shared_ptr<VarRec> varRec;
  ReturnReason* reason;

  bool localCount(std::string);
  void setOrCreateVar(std::string, Literal* expr);

 public:
  ExecContext(ExecContext* parent = nullptr, ReturnReason* reason = nullptr)
      : parent(parent), reason(reason) {
    varRec = std::make_shared<VarRec>();
  }

  bool count(std::string);

  void define(std::string, Literal* expr);
  void set(std::string, Literal* expr);
  Literal* get(std::string);

  void setReason(ReturnReason r);
  ReturnReason getReason();

  bool isWrapped() const { return parent; }

  ExecContext wrap() { return ExecContext(this); }
};
