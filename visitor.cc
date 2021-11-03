#include "visitor.h"

#include <iostream>
#include <string>

#include "ast.h"
#include "llvm.h"
#include "log.h"

CodeGenVisitor CodeGenVisitor::wrap() {
  return CodeGenVisitor(scope.wrap(), l);
}
CodeGenVisitor CodeGenVisitor::wrapWithTrace(Trace* r) {
  return CodeGenVisitor(scope.wrapWithTrace(r), l);
}

void CodeGenExprVisitor::visit(Expr* expr) { expr->accept(this); }

void CodeGenExprVisitor::visit(Literal* expr) { expr->accept(this); }

void CodeGenExprVisitor::visit(Integer* expr) {
  setValue(llvm::ConstantInt::get(*l.ctx, llvm::APInt(32, expr->value)));
}

void CodeGenExprVisitor::visit(Double* expr) {
  setValue(llvm::ConstantFP::get(*l.ctx, llvm::APFloat(expr->value)));
}

void CodeGenExprVisitor::visit(Binary* expr) {
  CodeGenExprVisitor lv(scope, l);
  lv.visit(expr->left);
  auto lhs = lv.getValue();

  CodeGenExprVisitor rv(scope, l);
  rv.visit(expr->right);
  auto rhs = rv.getValue();

  auto op = expr->op.tokenType;

  bool hasDouble = false;
  bool hasInteger = false;

  if (lhs) {
    hasDouble = lhs->getType()->isDoubleTy() || rhs->getType()->isDoubleTy();
    hasInteger = lhs->getType()->isIntegerTy() || rhs->getType()->isIntegerTy();
  }
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
        setTuple(l.builder->CreateFAdd(lhs, rhs));
      else if (hasInteger)
        setTuple(l.builder->CreateAdd(lhs, rhs));
      else
        abortMsg("type mismatched");
      setAddr(nullptr);
      break;
    case MINUS:
      if (hasDouble)
        setTuple(l.builder->CreateFSub(lhs, rhs));
      else if (hasInteger)
        setTuple(l.builder->CreateSub(lhs, rhs));
      else
        abortMsg("type mismatched");
      break;
    case STAR:
      if (hasDouble)
        setTuple(l.builder->CreateFMul(lhs, rhs));
      else if (hasInteger)
        setTuple(l.builder->CreateMul(lhs, rhs));
      else
        abortMsg("type mismatched");
      break;
    case SLASH:
      if (hasDouble)
        setTuple(l.builder->CreateFDiv(lhs, rhs));
      else if (hasInteger)
        setTuple(l.builder->CreateSDiv(lhs, rhs));
      else
        abortMsg("type mismatched");
      break;
    case LESS:
      if (hasDouble)
        setTuple(l.builder->CreateFCmpOLT(lhs, rhs));
      else if (hasInteger)
        setTuple(l.builder->CreateICmpSLT(lhs, rhs));
      else
        abortMsg("type mismatched");
      break;
    case EQUAL:
      if (lv.getAddr()) {
        l.builder->CreateStore(rhs, lv.getAddr());
        setTuple(rhs);
      } else
        abortMsg("cannot assign value to rvalue");
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
  auto r = scope.get(expr->name);
  addr = r.addr;
  value = l.builder->CreateLoad(l.getType(r.type), r.addr, r.id.c_str());
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
void CodeGenVisitor::visit(VarDecl* st) {
  auto type = l.getType(st->type);
  auto addr =
      l.createEntryBlockAlloca(scope.getTrace().llvmFun, type, st->identifier);
  scope.define(st->identifier, {st->identifier, st->type, addr});
}
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

  // Create a new basic block to start insertion into.
  llvm::BasicBlock* BB = llvm::BasicBlock::Create(*l.ctx, st->identifier, F);
  l.builder->SetInsertPoint(BB);

  Trace r = {F, st, nullptr};
  auto v = wrapWithTrace(&r);

  // Set names for all arguments.
  size_t i = 0;
  for (auto& a : F->args()) {
    FormalArg formal = st->args[i++];
    std::string name = formal.token.lexeme;
    a.setName(name);
    auto addr = l.createEntryBlockAlloca(F, l.getType(formal.type),
                                         formal.token.lexeme.c_str());
    l.builder->CreateStore(&a, addr);
    v.scope.define(name, {name, formal.type, addr});
  }

  v.visit(st->body);
  if (llvm::verifyFunction(*F, &llvm::errs())) abortMsg("verify error");
  // F->eraseFromParent();
}
void CodeGenVisitor::visit(BlockStmt* st) {
  auto v = wrap();
  for (auto d : st->decls) v.visit(d);
}
void CodeGenVisitor::visit(IfStmt* st) {
  CodeGenExprVisitor v(scope, l);
  v.visit(st->condition);
  llvm::Value* condV = v.getValue();
  if (!condV) abortMsg("failed to generate condition");

  condV = l.convertToTruthy(condV);
  llvm::Function* fun = l.builder->GetInsertBlock()->getParent();

  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*l.ctx, "then", fun);
  llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*l.ctx, "else");
  llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*l.ctx, "ifcont");

  l.builder->CreateCondBr(condV, thenBB, elseBB);
  // Emit then value.
  l.builder->SetInsertPoint(thenBB);

  auto v1 = wrap();
  v1.visit(st->true_branch);

  l.builder->CreateBr(mergeBB);
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  thenBB = l.builder->GetInsertBlock();

  // Emit else block.
  fun->getBasicBlockList().push_back(elseBB);
  l.builder->SetInsertPoint(elseBB);

  if (st->false_branch) {
    auto v2 = wrap();
    v2.visit(st->false_branch);
  }

  l.builder->CreateBr(mergeBB);
  // codegen of 'Else' can change the current block, update ElseBB for the PHI.
  elseBB = l.builder->GetInsertBlock();

  // Emit merge block.
  fun->getBasicBlockList().push_back(mergeBB);
  l.builder->SetInsertPoint(mergeBB);
}
void CodeGenVisitor::visit(WhileStmt* st) { unimplemented(); }
void CodeGenVisitor::visit(BreakStmt* st) { unimplemented(); }
void CodeGenVisitor::visit(ReturnStmt* st) {
  CodeGenExprVisitor v(scope, l);
  v.visit(st->expr);
  auto val =
      l.implictConvert(v.getValue(), scope.getTrace().llvmFun->getReturnType());
  l.builder->CreateRet(val);
}
