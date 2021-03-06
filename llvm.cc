#include "llvm.h"

#include "log.h"

llvm::Type* llvmWrapper::getBool() { return llvm::Type::getInt1Ty(*ctx); }
llvm::Type* llvmWrapper::getInt() { return llvm::Type::getInt32Ty(*ctx); }
llvm::Type* llvmWrapper::getChar() { return llvm::Type::getInt8Ty(*ctx); }
llvm::Type* llvmWrapper::getDouble() { return llvm::Type::getDoubleTy(*ctx); }
llvm::Type* llvmWrapper::getVoid() { return llvm::Type::getVoidTy(*ctx); }

llvm::Type* llvmWrapper::getType(Type type) {
  auto baseType = getBaseType(type);
  if (type.isArray)
    return llvm::ArrayType::get(baseType, type.arraySize);
  else if (type.isPointer)
    return baseType->getPointerTo();
  else
    return baseType;
}

llvm::Type* llvmWrapper::getBaseType(Type type) {
  switch (type.base) {
    case Type::Base::INT:
      return getInt();
      break;
    case Type::Base::DOUBLE:
      return getDouble();
      break;
    case Type::Base::BOOL:
      return getBool();
      break;
    case Type::Base::CHAR:
      return getChar();
      break;
    case Type::Base::VOID:
      return getVoid();
    default:
      abortMsg("Unrecognize type");
  }
  return nullptr;
}

llvm::Value* llvmWrapper::convertToTruthy(llvm::Value* v) {
  auto t = v->getType();
  if (t == getBool()) return v;
  if (t->isIntegerTy()) {
    int w = t->getIntegerBitWidth();
    return builder->CreateICmpNE(
        v, llvm::ConstantInt::get(llvm::IntegerType::get(*ctx, w),
                                  llvm::APInt(w, 0, true)));
  }
  abortMsg("can't use value as boolean");
  return nullptr;
}

/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
/// the function.  This is used for mutable variables etc.
llvm::AllocaInst* llvmWrapper::createEntryBlockAlloca(llvm::Function* fun,
                                                      llvm::Type* type,
                                                      const std::string& name,
                                                      llvm::Value* num) {
  llvm::IRBuilder<> TmpB(&fun->getEntryBlock(), fun->getEntryBlock().begin());
  return TmpB.CreateAlloca(type, num, name.c_str());
}

llvm::Value* llvmWrapper::implictConvert(llvm::Value* v, llvm::Type* t) {
  if (v->getType() == t) return v;
  if (t->isDoubleTy()) {
    return builder->CreateSIToFP(v, t, "todouble");
  } else if (t == getBool()) {
    return convertToTruthy(v);
  } else if (t->isIntegerTy()) {
    if (!v->getType()->isIntegerTy())
      abortMsg("can't implict convert double into int");
    else
      return builder->CreateIntCast(v, t, true, "toint");
  } else if (t->isArrayTy()) {
    return v;
  } else if (t->isPointerTy()) {
    return builder->CreateGEP(v, 0);
  }
  abortMsg("unimplemented implict convert");
  return nullptr;
}
