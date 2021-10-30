#pragma once

#include <map>
#include <memory>
#include <string>

class Expr;
enum class ReturnReason { NORMAL, RETURN, BREAK, CONTINUE };
class ExecContext {
  ExecContext* parent;
  typedef std::map<std::string, Expr*> Rec;
  std::shared_ptr<Rec> rec;
  ReturnReason* reason;

  bool localCount(std::string);
  void setOrCreateVar(std::string, Expr* expr);

 public:
  ExecContext(ExecContext* parent = nullptr, ReturnReason* reason = nullptr)
      : parent(parent), reason(reason) {
    rec = std::make_shared<Rec>();
  }

  bool count(std::string);

  void define(std::string, Expr* expr);
  void set(std::string, Expr* expr);
  Expr* get(std::string);

  void setReason(ReturnReason r);
  ReturnReason getReason();

  bool isWrapped() const { return parent; }
};
