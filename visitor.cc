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

void PrintVisitor::visit(Declaration* d) { std::cout << (string)*d; }
void PrintVisitor::visit(ExprStmt* d) { std::cout << (string)*d; }
void PrintVisitor::visit(AssertStmt* d) { std::cout << (string)*d; }
void PrintVisitor::visit(PrintStmt* d) { std::cout << (string)*d; }
void PrintVisitor::visit(VarDecl* d) { std::cout << (string)*d; }
void PrintVisitor::visit(FunDecl* d) { std::cout << (string)*d; }
void PrintVisitor::visit(BlockStmt* d) { std::cout << (string)*d; }
void PrintVisitor::visit(IfStmt* d) { std::cout << (string)*d; }
void PrintVisitor::visit(WhileStmt* d) { std::cout << (string)*d; }
void PrintVisitor::visit(BreakStmt* d) { std::cout << (string)*d; }
void PrintVisitor::visit(ReturnStmt* d) { std::cout << (string)*d; }
