#include "visitor.h"

#include <iostream>
#include <string>

#include "ast.h"
using std::string;
void PrintExprVisitor::visit(Expr* expr) { std::cout << (string)*expr; }
void PrintExprVisitor::visit(Literal* expr) { std::cout << (string)*expr; }
void PrintExprVisitor::visit(Unary* expr) { std::cout << (string)*expr; }
void PrintExprVisitor::visit(Binary* expr) { std::cout << (string)*expr; }
void PrintExprVisitor::visit(Variable* expr) { std::cout << (string)*expr; }
