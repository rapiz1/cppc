#include "expr.h"

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
    default:
      throw RuntimeError();
      break;
  }
}

void EvalVisitor::visit(Variable* expr) {
  if (!context.count(expr->name)) {
    std::cerr << "Cannot find var " << expr->name << " in the context\n";
  }
  value = context.get(expr->name);
}

Number::Number(Token token) {
  std::stringstream ss(token.lexeme);
  ss >> value;
}

Boolean::operator std::string() { return value ? "true" : "false"; }

bool String::isTruthy() { return value.size(); }

const double EPS = 1e-6;
bool Number::isTruthy() { return abs(value) < EPS; }

void ExecVisitor::visit(Declaration* s) { s->accept(this); }
void ExecVisitor::visit(PrintStmt* s) {
  EvalVisitor v(context);
  v.visit(s->expr);
  Expr* e = v.getValue();

  std::cout << std::string(*e) << std::endl;
}
void ExecVisitor::visit(ExprStmt* s) {}
void ExecVisitor::visit(VarDecl* s) {
  EvalVisitor v(context);
  v.visit(s->init);
  assert(v.getValue() != nullptr);
  context.set(s->identifier, v.getValue());
}
