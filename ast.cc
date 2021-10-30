#include "ast.h"

#include <string.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>

#include "exception.h"
using std::string;

Binary::operator std::string() {
  return "(" + string(*left) + op.lexeme + string(*right) + ")";
}

Unary::operator std::string() { return op.lexeme + string(*child); }

Number::operator std::string() {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

String::operator std::string() { return "\"" + value + "\""; }

void PrintVisitor::visit(Expr* expr) { std::cout << (string)*expr; }
void PrintVisitor::visit(Literal* expr) { std::cout << (string)*expr; }
void PrintVisitor::visit(Unary* expr) { std::cout << (string)*expr; }
void PrintVisitor::visit(Binary* expr) { std::cout << (string)*expr; }
void PrintVisitor::visit(Variable* expr) { std::cout << (string)*expr; }

void EvalVisitor::visit(Expr* expr) { expr->accept(this); }
void EvalVisitor::visit(Literal* expr) { value = expr; }
void EvalVisitor::visit(Unary* expr) {
  EvalVisitor v(context);
  v.visit(expr->child);
  Expr* e = v.value;
  switch (expr->op.tokenType) {
    case MINUS:
      value = new Number(-dynamic_cast<Number*>(e)->value);
      break;
    case BANG:
      value = new Boolean(dynamic_cast<Literal*>(e)->isTruthy());
      break;
    default:
      throw RuntimeError();
      break;
  }
}

void EvalVisitor::visit(Call* call) {
  EvalVisitor v(context);
  v.visit(call->callee);
  Function* fun = dynamic_cast<Function*>(v.getValue());
  if (fun == nullptr) {
    std::cerr << "call on non-callable object";
    exit(-1);
  }
  FunDecl* d = fun->fun;
  auto formal = d->getArgs();
  auto real = call->args;
  if (formal.size() != real.size()) {
    std::cerr << "call with wrong arugments, expected " << formal.size()
              << " , got " << real.size();
    exit(-1);
  }

  ExecContext ctx = context.wrapWithReason(new ReturnResult());
  ExecVisitor ev(ctx);

  for (size_t i = 0; i < formal.size(); i++) {
    v.visit(real[i]);
    auto value = v.getValue();
    ctx.define(formal[i].lexeme, value);
  }
  ev.visit(d->getBody());

  auto r = ctx.getReason();
  assert(r.reason == ReturnReason::RETURN || r.reason == ReturnReason::NORMAL);
  value = r.value;
}

void EvalVisitor::visit(Binary* expr) {
  EvalVisitor v1(context), v2(context);
  v1.visit(expr->left);
  v2.visit(expr->right);
  Number* n1 = dynamic_cast<Number*>(v1.value);
  Number* n2 = dynamic_cast<Number*>(v2.value);
  String* s1 = dynamic_cast<String*>(v1.value);
  String* s2 = dynamic_cast<String*>(v2.value);
  Boolean* b1 = dynamic_cast<Boolean*>(v1.value);
  Boolean* b2 = dynamic_cast<Boolean*>(v2.value);
  Variable* lv = dynamic_cast<Variable*>(expr->left);
  Literal* rv = v2.value;
  switch (expr->op.tokenType) {
    case PLUS:
      if (n1 && n2) {
        value = new Number(n1->value + n2->value);
      } else if (s1 && s2) {
        value = new String(s1->value + s2->value);
      } else {
        throw TypeError();
      }
      break;
    case MINUS:
      if (n1 && n2) {
        value = new Number(n1->value - n2->value);
      } else {
        throw TypeError();
      }
      break;
    case STAR:
      if (n1 && n2) {
        value = new Number(n1->value * n2->value);
      } else {
        throw TypeError();
      }
      break;
    case SLASH:
      if (n1 && n2) {
        value = new Number(n1->value / n2->value);
      } else {
        throw TypeError();
      }
      break;
    case EQUAL:
      if (lv->isLval() && !rv->isLval()) {
        context.set(lv->name, rv);
      } else {
        throw BindError();
      }
      break;
    case EQUAL_EQUAL:
      if (n1 && n2) {
        value = new Boolean(n1->value == n2->value);
      } else if (b1 && b2) {
        value = new Boolean(b1->value == b2->value);
      } else {
        throw TypeError();
      }
      break;
    case BANG_EQUAL:
      if (n1 && n2) {
        value = new Boolean(n1->value != n2->value);
      } else if (b1 && b2) {
        value = new Boolean(b1->value != b2->value);
      } else {
        throw TypeError();
      }
      break;
    case LESS:
      if (n1 && n2) {
        value = new Boolean(n1->value < n2->value);
      } else {
        throw TypeError();
      }
      break;
    case GREATER:
      if (n1 && n2) {
        value = new Boolean(n1->value > n2->value);
      } else {
        throw TypeError();
      }
      break;
    default:
      throw RuntimeError();
      break;
  }
}

void EvalVisitor::visit(Variable* expr) {
  if (!context.count(expr->name)) {
    std::cerr << "Cannot find var " << expr->name << " in the context\n";
    exit(-1);
  }
  value = dynamic_cast<Literal*>(context.get(expr->name));
  assert(value);
}

Number::Number(Token token) {
  std::stringstream ss(token.lexeme);
  ss >> value;
}

Boolean::operator std::string() { return value ? "true" : "false"; }

bool String::isTruthy() { return value.size(); }

const double EPS = 1e-6;
bool Number::isTruthy() { return abs(value) < EPS; }

ExecVisitor ExecVisitor::wrap() { return ExecVisitor(context.wrap()); }
ExecVisitor ExecVisitor::wrapWithReason() {
  return ExecVisitor(context.wrapWithReason(new ReturnResult()));
}

void ExecVisitor::visit(Declaration* s) { s->accept(this); }
void ExecVisitor::visit(PrintStmt* s) {
  EvalVisitor v(context);
  v.visit(s->expr);
  Expr* e = v.getValue();

  std::cout << std::string(*e) << std::endl;
}
void ExecVisitor::visit(ExprStmt* s) {
  EvalVisitor v(context);
  v.visit(s->expr);
}
void ExecVisitor::visit(VarDecl* s) {
  EvalVisitor v(context);
  v.visit(s->init);
  assert(v.getValue() != nullptr);
  context.define(s->identifier, v.getValue());
}

void ExecVisitor::visit(FunDecl* s) {
  if (context.count(s->identifier)) {
    std::cerr << "redefine function " << s->identifier;
    exit(-1);
  }
  context.define(s->identifier, new Function(s));
}

void ExecVisitor::visit(BlockStmt* s) {
  ExecVisitor v = wrap();
  for (auto d : s->decls) {
    v.visit(d);
    if (v.context.getReason().reason != ReturnReason::NORMAL) break;
  }
}

void ExecVisitor::visit(IfStmt* s) {
  EvalVisitor ev(context);
  ev.visit(s->condition);
  Literal* l = ev.getValue();
  assert(l);

  ExecVisitor v = wrap();
  if (l->isTruthy()) {
    v.visit(s->true_branch);
  } else if (s->false_branch) {
    v.visit(s->false_branch);
  }
}

void ExecVisitor::visit(WhileStmt* s) {
  ExecVisitor v = wrapWithReason();
  EvalVisitor ev(context);
  while (1) {
    ev.visit(s->condition);
    Literal* l = ev.getValue();
    assert(l);
    if (!l->isTruthy()) break;
    v.visit(s->body);
    auto r = v.context.getReason().reason;
    if (r == ReturnReason::BREAK || r == ReturnReason::RETURN) break;
  }
}

void ExecVisitor::visit(BreakStmt* s) {
  context.setReason({ReturnReason::BREAK});
}

void ExecVisitor::visit(ReturnStmt* s) {
  if (!s->expr) return;
  EvalVisitor ev(context);
  ev.visit(s->expr);
  context.setReason({ReturnReason::RETURN, ev.getValue()});
}
