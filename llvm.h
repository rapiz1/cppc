#pragma once

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "type.h"

struct llvmWrapper {
  std::shared_ptr<llvm::LLVMContext> ctx;
  std::shared_ptr<llvm::Module> mod;
  std::shared_ptr<llvm::IRBuilder<>> builder;
  llvmWrapper() {
    ctx = std::make_shared<llvm::LLVMContext>();
    mod = std::make_shared<llvm::Module>("mod", *ctx);
    builder = std::make_shared<llvm::IRBuilder<>>(*ctx);
  };
  llvm::Type* getType(Type t);
  llvm::Value* convertToTruthy(llvm::Value*);
  llvm::Value* implictConvert(llvm::Value*, llvm::Type*);
  llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* fun,
                                           llvm::Type* type,
                                           const std::string& name);
};
