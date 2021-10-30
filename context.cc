#include "context.h"

#include <cassert>
#include <iostream>

#include "cmdargs.h"
using std::string;

bool ExecContext::localCount(string name) { return varRec->count(name); }
bool ExecContext::count(string name) {
  if (localCount(name))
    return true;
  else if (parent)
    return parent->count(name);
  return false;
}

void ExecContext::define(string name, Literal* expr) {
  if (localCount(name) && options->getAllowRedefine()) {
    std::cerr << "redefine " << name << std::endl;
    exit(-1);
  } else
    setOrCreateVar(name, expr);
}

void ExecContext::setOrCreateVar(string name, Literal* expr) {
  (*varRec)[name] = expr;
}

void ExecContext::set(string name, Literal* expr) {
  if (localCount(name))
    setOrCreateVar(name, expr);
  else if (parent && parent->count(name))
    parent->set(name, expr);
  else {
    std::cerr << "Cannot set undefined variable " << name << std::endl;
    exit(-1);
  }
}

Literal* ExecContext::get(string name) {
  if (localCount(name)) {
    auto ret = (*varRec)[name];
    assert(ret);
    return ret;
  } else if (parent) {
    return parent->get(name);
  } else {
    std::cerr << "Cannot get undefined variable " << name << std::endl;
    exit(-1);
  }
  return nullptr;
}

ReturnReason ExecContext::getReason() {
  if (reason) {
    return *reason;
  } else if (parent) {
    return parent->getReason();
  } else {
    return ReturnReason::NORMAL;
  }
}

void ExecContext::setReason(ReturnReason r) {
  if (reason) {
    *reason = r;
  } else if (parent) {
    parent->setReason(r);
  } else {
    std::cerr << "Return in an invalid context";
    exit(-1);
  }
}
