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
void CodeGenExprVisitor::visit(Boolean* expr) {
  setValue(llvm::ConstantInt::get(*l.ctx, llvm::APInt(1, expr->value)));
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
    unsigned int maxw = 0;
    if (lhs->getType()->isIntegerTy())
      maxw = std::max(maxw, lhs->getType()->getIntegerBitWidth());
    if (rhs->getType()->isIntegerTy())
      maxw = std::max(maxw, rhs->getType()->getIntegerBitWidth());
    auto upgradeType = llvm::IntegerType::get(*l.ctx, maxw);
    if (lhs->getType() != upgradeType)
      lhs = l.builder->CreateIntCast(lhs, upgradeType, true, "casttmp");
    if (rhs->getType() != upgradeType)
      rhs = l.builder->CreateIntCast(rhs, upgradeType, true, "casttmp");
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
    case LESS_EQUAL:
      if (hasDouble)
        setTuple(l.builder->CreateFCmpOLE(lhs, rhs));
      else if (hasInteger)
        setTuple(l.builder->CreateICmpSLE(lhs, rhs));
      else
        abortMsg("type mismatched");
      break;
    case GREATER:
      if (hasDouble)
        setTuple(l.builder->CreateFCmpOGT(lhs, rhs));
      else if (hasInteger)
        setTuple(l.builder->CreateICmpSGT(lhs, rhs));
      else
        abortMsg("type mismatched");
      break;
    case GREATER_EQUAL:
      if (hasDouble)
        setTuple(l.builder->CreateFCmpOGE(lhs, rhs));
      else if (hasInteger)
        setTuple(l.builder->CreateICmpSGE(lhs, rhs));
      else
        abortMsg("type mismatched");
      break;
    case EQUAL_EQUAL:
      if (hasDouble)
        setTuple(l.builder->CreateFCmpOEQ(lhs, rhs));
      else if (hasInteger)
        setTuple(l.builder->CreateICmpEQ(lhs, rhs));
      else
        abortMsg("type mismatched");
      break;
    case BANG_EQUAL:
      if (hasDouble)
        setTuple(l.builder->CreateFCmpONE(lhs, rhs));
      else if (hasInteger)
        setTuple(l.builder->CreateICmpNE(lhs, rhs));
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
  visit(expr->child);
  auto op = expr->op.tokenType;
  if (op == BANG) {
    value = l.convertToTruthy(value);
    value = l.builder->CreateNot(value);
  } else if (op == MINUS) {
    value = l.builder->CreateNeg(value);
  } else {
    if (!addr)
      abortMsg("cannot apply operator " + expr->op.lexeme + " to lvalue");
    if (!value->getType()->isIntegerTy())
      abortMsg("cant apply " + expr->op.lexeme + "to non integer");
    int width = value->getType()->getIntegerBitWidth();
    auto con = llvm::Constant::getIntegerValue(value->getType(),
                                               llvm::APInt(width, 1, true));
    switch (op) {
      case MINUSMINUS:
        value = l.builder->CreateSub(value, con);
        break;
      case PLUSPLUS:
        value = l.builder->CreateAdd(value, con);
        break;
      default:
        break;
    }
    l.builder->CreateStore(addr, value);
  }
}

void CodeGenExprVisitor::visit(Postfix* expr) {
  visit(expr->child);
  auto val = getValue();
  auto addr = getAddr();
  if (!val->getType()->isIntegerTy())
    abortMsg("cant apply " + expr->op.lexeme + "to non integer");
  int width = val->getType()->getIntegerBitWidth();
  auto con = llvm::Constant::getIntegerValue(val->getType(),
                                             llvm::APInt(width, 1, true));
  llvm::Value* ret = nullptr;
  switch (expr->op.tokenType) {
    case PLUSPLUS:
      ret = l.builder->CreateAdd(val, con);
      break;
    case MINUSMINUS:
      ret = l.builder->CreateSub(val, con);
      break;
    default:
      abortMsg("unimplemented postfix operator " + expr->op.lexeme);
      break;
  }
  l.builder->CreateStore(ret, addr);
}
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
void CodeGenVisitor::visit(AssertStmt* st) {
  CodeGenExprVisitor v(scope, l);
  v.visit(st->expr);
  // FIXME:
}
void CodeGenVisitor::visit(PrintStmt* st) {
  CodeGenExprVisitor v(scope, l);
  v.visit(st->expr);
  // FIXME:
}
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

  Trace r = {F, st};
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
  if (llvm::verifyFunction(*F, &llvm::errs()))
    ;  // abortMsg("verify error");
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
  auto v1 = wrap();
  l.builder->SetInsertPoint(thenBB);
  v1.visit(st->true_branch);
  if (!v1.terminate) l.builder->CreateBr(mergeBB);

  // Emit else block.
  fun->getBasicBlockList().push_back(elseBB);
  l.builder->SetInsertPoint(elseBB);

  bool else_terminate = false;
  if (st->false_branch) {
    auto v2 = wrap();
    v2.visit(st->false_branch);
    else_terminate = v2.terminate;
  }
  if (!else_terminate) l.builder->CreateBr(mergeBB);

  // codegen of 'Else' can change the current block, update ElseBB for the PHI.
  elseBB = l.builder->GetInsertBlock();

  // Emit merge block.
  fun->getBasicBlockList().push_back(mergeBB);
  l.builder->SetInsertPoint(mergeBB);
}
void CodeGenVisitor::visit(WhileStmt* st) {
  auto f = l.builder->GetInsertBlock()->getParent();
  auto beginB = llvm::BasicBlock::Create(*l.ctx, "loopBegin", f);
  auto bodyB = llvm::BasicBlock::Create(*l.ctx, "loopBody");
  auto endB = llvm::BasicBlock::Create(*l.ctx, "loopEnd");

  l.builder->CreateBr(beginB);

  // emit the condittion
  CodeGenExprVisitor v(scope, l);
  l.builder->SetInsertPoint(beginB);
  v.visit(st->condition);
  l.builder->CreateCondBr(v.getValue(), bodyB, endB);

  // emit the body
  auto t = scope.getTrace();
  t.endB = endB;
  t.contB = beginB;
  auto v1 = wrapWithTrace(&t);
  f->getBasicBlockList().push_back(bodyB);
  l.builder->SetInsertPoint(bodyB);
  v1.visit(st->body);
  if (!v1.terminate) l.builder->CreateBr(beginB);

  // set inserter to endB
  l.builder->SetInsertPoint(endB);
  f->getBasicBlockList().push_back(endB);
}
void CodeGenVisitor::visit(BreakStmt* st) {
  auto t = scope.getTrace();
  auto endB = t.endB;

  terminate = true;

  if (!endB) abortMsg("break in non-loop");
  l.builder->CreateBr(endB);
}
void CodeGenVisitor::visit(ReturnStmt* st) {
  auto t = scope.getTrace();

  terminate = true;

  CodeGenExprVisitor v(scope, l);
  v.visit(st->expr);
  auto val = l.implictConvert(v.getValue(), t.llvmFun->getReturnType());
  l.builder->CreateRet(val);
}
