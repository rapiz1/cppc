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
