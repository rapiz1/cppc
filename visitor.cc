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
void CodeGenExprVisitor::visit(Char* expr) {
  setValue(llvm::ConstantInt::get(*l.ctx, llvm::APInt(8, expr->value)));
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

  if (op != EQUAL) {
    if (lhs) {
      hasDouble = lhs->getType()->isDoubleTy() || rhs->getType()->isDoubleTy();
      hasInteger =
          lhs->getType()->isIntegerTy() || rhs->getType()->isIntegerTy();
    }
    if (hasDouble) {
      if (!lhs->getType()->isDoubleTy())
        lhs = l.builder->CreateFPCast(lhs, llvm::Type::getDoubleTy(*l.ctx),
                                      "casttmp");
      if (!rhs->getType()->isDoubleTy())
        rhs = l.builder->CreateFPCast(rhs, llvm::Type::getDoubleTy(*l.ctx),
                                      "casttmp");
    } else if (hasInteger) {  // integer upgrade
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
    case PERCENT:
      if (hasInteger) {
        setTuple(l.builder->CreateSRem(lhs, rhs));
      } else
        abortMsg("cannot apply operator % on non-integer type");
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
    l.builder->CreateStore(value, addr);
  }
}

void CodeGenExprVisitor::visit(Postfix* expr) {
  visit(expr->child);
  if (!value->getType()->isIntegerTy())
    abortMsg("cant apply " + expr->op.lexeme + "to non integer");
  int width = value->getType()->getIntegerBitWidth();
  auto con = llvm::Constant::getIntegerValue(value->getType(),
                                             llvm::APInt(width, 1, true));
  llvm::Value* ret = nullptr;
  switch (expr->op.tokenType) {
    case PLUSPLUS:
      ret = l.builder->CreateAdd(value, con);
      break;
    case MINUSMINUS:
      ret = l.builder->CreateSub(value, con);
      break;
    default:
      abortMsg("unimplemented postfix operator " + expr->op.lexeme);
      break;
  }
  l.builder->CreateStore(ret, addr);
}
void CodeGenExprVisitor::visit(String* expr) {
  auto s = expr->value;
  int size = s.size() + 1;
  addr = nullptr;
  value = l.createEntryBlockAlloca(
      scope.getTrace().llvmFun, l.getChar(), s,
      llvm::Constant::getIntegerValue(l.getInt(), llvm::APInt(32, size)));
  for (int i = 0; i <= s.size(); i++) {
    char c = 0;
    if (i < s.size()) c = s[i];
    auto ptr = l.builder->CreateConstGEP1_32(value, i, "idx");
    l.builder->CreateStore(
        llvm::Constant::getIntegerValue(l.getChar(), llvm::APInt(8, c)), ptr);
  }
}
void CodeGenExprVisitor::visit(Variable* expr) {
  auto r = scope.get(expr->name);
  if (r.type.isArray) {
    addr = nullptr;  // an array is a lvalue
    value = r.addr;  // value of an array is its base address
  } else {
    addr = r.addr;
    value = l.builder->CreateLoad(l.getType(r.type), r.addr, r.id.c_str());
  }
}
void CodeGenExprVisitor::visit(Index* expr) {
  CodeGenExprVisitor ev(scope, l);
  ev.visit(expr->base);
  CodeGenExprVisitor ev2(scope, l);
  ev2.visit(expr->idx);
  auto idx = ev2.getValue();
  auto base = ev.getValue();
  auto ptr = l.builder->CreateGEP(base, idx);
  value = l.builder->CreateLoad(ptr);
  addr = static_cast<llvm::AllocaInst*>(ptr);
}
void CodeGenExprVisitor::visit(Call* expr) {
  // Look up the name in the global module table.
  auto funcName = dynamic_cast<Variable*>(expr->callee)->name;
  llvm::Function* fun = l.mod->getFunction(funcName);
  if (!fun) abortMsg("Unknown function " + funcName + " referenced");

  // If argument mismatch error.
  if (fun->arg_size() != expr->args.size())
    abortMsg("Incorrect # arguments passed");

  std::vector<llvm::Value*> args;

  CodeGenExprVisitor v(scope, l);

  size_t i = 0;
  for (auto a : expr->args) {
    auto protoArg = fun->getArg(i++);
    v.visit(a);
    auto val = v.getValue();
    if (protoArg->getType()->isPointerTy()) {
      // FIXME: a hack. see all array type in function prototype as ptrs
    } else {
      val = l.implictConvert(val, protoArg->getType());
    }
    args.push_back(val);
    if (!args.back()) {
      value = nullptr;
      abortMsg("failed to generate for arguments");
    }
  }

  value = l.builder->CreateCall(fun, args);
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

  llvm::Value* size = nullptr;

  if (st->type.isArray) {
    size = llvm::Constant::getIntegerValue(l.getInt(),
                                           llvm::APInt(32, st->type.arraySize));
    type = type->getArrayElementType();
  }

  llvm::AllocaInst* addr = nullptr;
  if (st->init && st->type.isArray) {
    auto baseType = st->type.base;
    if (baseType == Type::Base::CHAR && dynamic_cast<String*>(st->init)) {
      CodeGenExprVisitor ev(scope, l);
      ev.visit(st->init);
      addr = (llvm::AllocaInst*)ev.getValue();
    } else {
      abortMsg("array doesn't not support this kind of initializers");
    }
  } else {
    addr = l.createEntryBlockAlloca(scope.getTrace().llvmFun, type,
                                    st->identifier, size);
    if (st->init) {
      CodeGenExprVisitor ev(scope, l);
      ev.visit(st->init);
      auto val = ev.getValue();
      val = l.implictConvert(val, type);
      l.builder->CreateStore(val, addr);
    }
  }
  scope.define(st->identifier, {st->identifier, st->type, addr});
}

void CodeGenVisitor::visit(FunDecl* st) {
  if (scope.isWrapped()) abortMsg("nested function is forbidden");
  llvm::Function* F = l.mod->getFunction(st->identifier);
  if (F && !F->empty()) abortMsg("redefine func");

  std::vector<llvm::Type*> args;
  for (auto [type, token] : st->args) {
    auto t = l.getType(type);
    if (t->isArrayTy()) {
      // FIXME: hack, see all array type in funct proto as ptrs
      t = t->getArrayElementType()->getPointerTo();
    }
    args.push_back(t);
  }

  llvm::FunctionType* FT =
      llvm::FunctionType::get(l.getType(st->retType), args, false);

  F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                             st->identifier, l.mod.get());
  if (st->body == nullptr) {  // a prototype
    size_t i = 0;
    for (auto& a : F->args()) {
      TypedVar formal = st->args[i++];
      std::string name = formal.id.lexeme;
      a.setName(name);
    }
    return;
  }

  // Create a new basic block to start insertion into.
  llvm::BasicBlock* BB = llvm::BasicBlock::Create(*l.ctx, st->identifier, F);
  l.builder->SetInsertPoint(BB);

  Trace r = {F, st};
  auto v = wrapWithTrace(&r);

  // Set names for all arguments.
  size_t i = 0;
  for (auto& a : F->args()) {
    TypedVar formal = st->args[i++];
    std::string name = formal.id.lexeme;
    a.setName(name);
    auto addr = l.createEntryBlockAlloca(F, l.getType(formal.type),
                                         formal.id.lexeme.c_str());
    l.builder->CreateStore(&a, addr);
    v.scope.define(name, {name, formal.type, addr});
  }

  v.visit(st->body);

  // add a "return 0" automatically if it's missing in function main
  if (!v.terminate && st->identifier == "main") {
    l.builder->CreateRet(
        llvm::Constant::getIntegerValue(l.getInt(), llvm::APInt(32, 0)));
  }
  if (llvm::verifyFunction(*F, &llvm::errs()))
    ;  // abortMsg("verify error");
  // F->eraseFromParent();
}
void CodeGenVisitor::visit(BlockStmt* st) {
  auto v = wrap();
  for (auto d : st->decls) {
    v.visit(d);
    if (v.terminate) {
      terminate = true;
      break;
    }
  }
}
void CodeGenVisitor::visit(IfStmt* st) {
  CodeGenExprVisitor v(scope, l);
  v.visit(st->condition);
  llvm::Value* condV = v.getValue();
  if (!condV) abortMsg("failed to generate condition");

  condV = l.convertToTruthy(condV);
  llvm::Function* fun = l.builder->GetInsertBlock()->getParent();

  // Create blocks for the then and else cases.  Insert the 'then' block at
  // the end of the function.
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

  // codegen of 'Else' can change the current block, update ElseBB for the
  // PHI.
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
  l.builder->CreateCondBr(l.convertToTruthy(v.getValue()), bodyB, endB);

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
  if (st->expr) {
    v.visit(st->expr);
    auto val = l.implictConvert(v.getValue(), t.llvmFun->getReturnType());
    l.builder->CreateRet(val);
  } else {
    l.builder->CreateRetVoid();
  }
}
