#include "context.h"
using std::string;

bool ExecContext::count(string name) { return rec->count(name); }
void ExecContext::set(string name, Expr* expr) { (*rec)[name] = expr; }
Expr* ExecContext::get(string name) {
  if (!count(name))
    return nullptr;
  else
    return (*rec)[name];
}
