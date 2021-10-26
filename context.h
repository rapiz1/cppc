#pragma once

#include <map>
#include <memory>
#include <string>

class Expr;

class ExecContext {
  ExecContext* parent;
  typedef std::map<std::string, Expr*> Rec;
  std::shared_ptr<Rec> rec;

  bool localCount(std::string);
  void setOrCreateVar(std::string, Expr* expr);

 public:
  ExecContext() : parent(nullptr) { rec = std::make_shared<Rec>(); }
  ExecContext(ExecContext* parent) : parent(parent) {
    rec = std::make_shared<Rec>();
  }
  bool count(std::string);
  void define(std::string, Expr* expr);
  void set(std::string, Expr* expr);
  Expr* get(std::string);
};
