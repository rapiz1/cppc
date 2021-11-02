#include "context.h"

#include <cassert>
#include <iostream>

#include "cmdargs.h"
#include "converter.h"
#include "log.h"
using std::string;

bool Scope::localCount(string name) { return varRec->count(name); }
bool Scope::count(string name) {
  if (localCount(name))
    return true;
  else if (parent)
    return parent->count(name);
  return false;
}

void Scope::define(string name, Record r) {
  if (localCount(name) && options->getAllowRedefine()) {
    std::cerr << "redefine " << name << std::endl;
    exit(-1);
  } else
    setOrCreateVar(name, r);
}

void Scope::setOrCreateVar(string name, Record r) { (*varRec)[name] = r; }

void Scope::set(string name, Record r) {
  if (localCount(name)) {
    if (get(name).type == r.type)
      setOrCreateVar(name, r);
    else
      abortMsg("type mismatched");
  } else if (parent && parent->count(name))
    parent->set(name, r);
  else {
    std::cerr << "Cannot set undefined variable " << name << std::endl;
    exit(-1);
  }
}

Record Scope::get(string name) {
  if (localCount(name)) {
    auto ret = (*varRec)[name];
    return ret;
  } else if (parent) {
    return parent->get(name);
  } else {
    std::cerr << "Cannot get undefined variable " << name << std::endl;
    exit(-1);
  }
}

ReturnResult Scope::getReason() {
  if (reason) {
    return *reason;
  } else if (parent) {
    return parent->getReason();
  } else {
    return ReturnResult{ReturnReason::NORMAL, nullptr};
  }
}

void Scope::setReason(ReturnResult r) {
  if (reason) {
    *reason = r;
  } else if (parent) {
    parent->setReason(r);
  } else {
    std::cerr << "Return in an invalid context";
    exit(-1);
  }
}
