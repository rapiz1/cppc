#pragma once

#include <map>
#include <memory>
#include <string>

class Expr;
class ExecContext {
  typedef std::map<std::string, Expr*> Rec;
  std::shared_ptr<Rec> rec;

 public:
  ExecContext() { rec = std::make_shared<Rec>(); }
  bool count(std::string);
  void set(std::string, Expr* expr);
  Expr* get(std::string);
};
