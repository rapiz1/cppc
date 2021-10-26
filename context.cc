#include "context.h"

#include <iostream>

#include "cmdargs.h"
using std::string;

bool ExecContext::localCount(string name) { return rec->count(name); }
bool ExecContext::count(string name) {
  if (localCount(name))
    return true;
  else if (parent)
    return parent->count(name);
  return false;
}

void ExecContext::define(string name, Expr* expr) {
  if (localCount(name) && options->getAllowRedefine()) {
    std::cerr << "redefine " << name << std::endl;
    exit(-1);
  } else
    setOrCreateVar(name, expr);
}

void ExecContext::setOrCreateVar(string name, Expr* expr) {
  (*rec)[name] = expr;
}

void ExecContext::set(string name, Expr* expr) {
  if (localCount(name))
    setOrCreateVar(name, expr);
  else if (parent && parent->count(name))
    parent->set(name, expr);
  else {
    std::cerr << "Cannot set undefined variable " << name << std::endl;
    exit(-1);
  }
}

Expr* ExecContext::get(string name) {
  if (localCount(name))
    return (*rec)[name];
  else if (parent) {
    return parent->get(name);
  } else {
    std::cerr << "Cannot get undefined variable " << name << std::endl;
    exit(-1);
  }
  return nullptr;
}
