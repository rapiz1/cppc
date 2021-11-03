#include "llvm.h"

#include "log.h"

llvm::Type* llvmWrapper::getType(Type type) {
  switch (type.base) {
    case Type::Base::INT:
      return llvm::Type::getInt32Ty(*ctx);
      break;
    case Type::Base::DOUBLE:
      return llvm::Type::getDoubleTy(*ctx);
      break;
    case Type::Base::BOOL:
    case Type::Base::CHAR:
      return llvm::Type::getInt8Ty(*ctx);
      break;
    default:
      abortMsg("Unrecognize type");
  }
  return nullptr;
}

llvm::Value* llvmWrapper::isTruthy(llvm::Value* v) {
  if (v->getType()->isIntegerTy())
    return builder->CreateICmpNE(
        v, llvm::ConstantInt::get(v->getType(), llvm::APInt(32, 0)));
  abortMsg("can't use value as boolean");
  return nullptr;
}

/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
/// the function.  This is used for mutable variables etc.
llvm::AllocaInst* llvmWrapper::createEntryBlockAlloca(llvm::Function* fun,
                                                      llvm::Type* type,
                                                      const std::string& name) {
  llvm::IRBuilder<> TmpB(&fun->getEntryBlock(), fun->getEntryBlock().begin());
  return TmpB.CreateAlloca(type, 0, name.c_str());
}

llvm::Value* llvmWrapper::convertTo(llvm::Value* v, llvm::Type* t) {
  if (v->getType() == t) return v;
  if (t->isDoubleTy()) {
    return builder->CreateSIToFP(v, t, "todouble");
  } else if (t->isIntegerTy()) {
    if (!v->getType()->isIntegerTy())
      abortMsg("can't implict convert double into int");
    else
      return builder->CreateIntCast(v, t, true, "toint32");
  } else
    abortMsg("unimplemented implict convert");
  return nullptr;
}
