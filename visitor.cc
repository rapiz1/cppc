#include "visitor.h"

#include <iostream>
#include <string>

#include "ast.h"
#include "llvm.h"
#include "log.h"

CodeGenVisitor CodeGenVisitor::wrap() {
  return CodeGenVisitor(scope.wrap(), l);
}
CodeGenVisitor CodeGenVisitor::wrapWithReason(ReturnResult* r) {
  return CodeGenVisitor(scope.wrapWithReason(r), l);
}

void CodeGenExprVisitor::visit(Expr* expr) { expr->accept(this); }

void CodeGenExprVisitor::visit(Literal* expr) { expr->accept(this); }

void CodeGenExprVisitor::visit(Integer* expr) {
  value = llvm::ConstantInt::get(*l.ctx, llvm::APInt(32, expr->value));
}

void CodeGenExprVisitor::visit(Double* expr) {
  value = llvm::ConstantFP::get(*l.ctx, llvm::APFloat(expr->value));
}

void CodeGenExprVisitor::visit(Binary* expr) {
  visit(expr->left);
  auto lhs = getValue();

  visit(expr->right);
  auto rhs = getValue();

  auto op = expr->op.tokenType;
  bool hasDouble = lhs->getType()->isDoubleTy() || rhs->getType()->isDoubleTy();
  bool hasInteger =
      lhs->getType()->isIntegerTy() || rhs->getType()->isIntegerTy();
  if (hasDouble) {
    if (!lhs->getType()->isDoubleTy())
      lhs = l.builder->CreateFPCast(lhs, llvm::Type::getDoubleTy(*l.ctx),
                                    "casttmp");
    if (!rhs->getType()->isDoubleTy())
      rhs = l.builder->CreateFPCast(rhs, llvm::Type::getDoubleTy(*l.ctx),
                                    "casttmp");
  } else if (hasInteger) {
    if (!lhs->getType()->isIntegerTy())
      lhs = l.builder->CreateIntCast(lhs, llvm::Type::getInt32Ty(*l.ctx), true,
                                     "casttmp");
    if (!rhs->getType()->isIntegerTy())
      rhs = l.builder->CreateIntCast(rhs, llvm::Type::getInt32Ty(*l.ctx), true,
                                     "casttmp");
  }
  switch (op) {
    case PLUS:
      if (hasDouble)
        value = l.builder->CreateFAdd(lhs, rhs);
      else if (hasInteger)
        value = l.builder->CreateAdd(lhs, rhs);
      else
        abortMsg("type mismatched");
      break;
    case MINUS:
      if (hasDouble)
        value = l.builder->CreateFSub(lhs, rhs);
      else if (hasInteger)
        value = l.builder->CreateSub(lhs, rhs);
      else
        abortMsg("type mismatched");
      break;
    case STAR:
      if (hasDouble)
        value = l.builder->CreateFMul(lhs, rhs);
      else if (hasInteger)
        value = l.builder->CreateMul(lhs, rhs);
      else
        abortMsg("type mismatched");
      break;
    case SLASH:
      if (hasDouble)
        value = l.builder->CreateFDiv(lhs, rhs);
      else if (hasInteger)
        value = l.builder->CreateSDiv(lhs, rhs);
      else
        abortMsg("type mismatched");
      break;
    case LESS:
      if (hasDouble)
        value = l.builder->CreateFCmpOLT(lhs, rhs);
      else if (hasInteger)
        value = l.builder->CreateICmpSLT(lhs, rhs);
      else
        abortMsg("type mismatched");
      break;
    default:
      abortMsg("unexpected binary operator " + expr->op.lexeme);
  }
}

void CodeGenExprVisitor::visit(Unary* expr) {
  unimplemented();
  /*
  visit(expr->child);
  auto v = getValue();
  auto op = expr->op.tokenType;
  switch (op) {
    case BANG:
      break;

    default:
      break;
  }
  */
}

void CodeGenExprVisitor::visit(Postfix* expr) { abortMsg("unimplemented"); }
void CodeGenExprVisitor::visit(Variable* expr) {
  value = scope.get(expr->name).arg;
}
void CodeGenExprVisitor::visit(Call* expr) {
  // Look up the name in the global module table.
  llvm::Function* fun =
      l.mod->getFunction(dynamic_cast<Variable*>(expr->callee)->name);
  if (!fun) abortMsg("Unknown function referenced");

  // If argument mismatch error.
  if (fun->arg_size() != expr->args.size())
    abortMsg("Incorrect # arguments passed");

  std::vector<llvm::Value*> args;

  CodeGenExprVisitor v(scope, l);

  for (auto a : expr->args) {
    v.visit(a);
    args.push_back(v.getValue());
    if (!args.back()) {
      value = nullptr;
      return;
    }
  }

  value = l.builder->CreateCall(fun, args, "calltmp");
}

void CodeGenVisitor::visit(Declaration* d) { d->accept(this); }
void CodeGenVisitor::visit(ExprStmt* st) {
  CodeGenExprVisitor v(scope, l);
  v.visit(st->expr);
}
void CodeGenVisitor::visit(AssertStmt* st) { unimplemented(); }
void CodeGenVisitor::visit(PrintStmt* st) { unimplemented(); }
void CodeGenVisitor::visit(VarDecl* st) { unimplemented(); }
void CodeGenVisitor::visit(FunDecl* st) {
  if (scope.isWrapped()) abortMsg("nested function is forbidden");
  llvm::Function* F = l.mod->getFunction(st->identifier);
  if (F) abortMsg("redefine func");
  std::vector<llvm::Type*> args;
  for (auto [type, token] : st->args) {
    auto t = l.getType(type);
    args.push_back(t);
  }
  llvm::FunctionType* FT =
      llvm::FunctionType::get(l.getType(st->retType), args, false);

  F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                             st->identifier, l.mod.get());

  ReturnResult r = {};
  auto v = wrapWithReason(&r);

  // Set names for all arguments.
  size_t i = 0;
  for (auto& a : F->args()) {
    FormalArg formal = st->args[i++];
    std::string name = formal.token.lexeme;
    a.setName(name);
    v.scope.define(name, {name, formal.type, &a});
  }

  // Create a new basic block to start insertion into.
  llvm::BasicBlock* BB = llvm::BasicBlock::Create(*l.ctx, st->identifier, F);
  l.builder->SetInsertPoint(BB);

  v.visit(st->body);
  if (r.value) {
    l.builder->CreateRet(r.value);
    verifyFunction(*F);
  }
  // F->eraseFromParent();
}
void CodeGenVisitor::visit(BlockStmt* st) {
  for (auto d : st->decls) visit(d);
}
void CodeGenVisitor::visit(IfStmt* st) { unimplemented(); }
void CodeGenVisitor::visit(WhileStmt* st) { unimplemented(); }
void CodeGenVisitor::visit(BreakStmt* st) { unimplemented(); }
void CodeGenVisitor::visit(ReturnStmt* st) {
  CodeGenExprVisitor v(scope, l);
  v.visit(st->expr);
  scope.setReason({ReturnReason::NORMAL, v.getValue()});
}
