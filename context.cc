#include "context.h"

#include <iostream>

#include "cmdargs.h"
using std::string;

bool ExecContext::count(string name) { return rec->count(name); }
void ExecContext::define(string name, Expr* expr) {
  if (count(name) && options->getAllowRedefine()) {
    std::cerr << "redefine " << name << std::endl;
    exit(-1);
  } else
    set(name, expr);
}
void ExecContext::set(string name, Expr* expr) { (*rec)[name] = expr; }
Expr* ExecContext::get(string name) {
  if (!count(name)) {
    std::cerr << "get undefined variable " << name << std::endl;
    exit(-1);
    return nullptr;
  } else
    return (*rec)[name];
}
