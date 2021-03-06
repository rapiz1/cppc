#pragma once
#include <string>
#include <vector>

#include "token.h"

struct Type {
  enum class Base { VOID, INT, DOUBLE, CHAR, ARRAY, BOOL, FUNCTION } base;
  int arraySize;
  std::vector<int> dims;
  bool isArray;
  bool isPointer;
  bool operator==(const Type& rhs) const {
    return base == rhs.base && arraySize == rhs.arraySize &&
           isArray == rhs.isArray;
  }
  operator std::string();
};

struct TypedVar {
  Type type;
  Token id;
};
