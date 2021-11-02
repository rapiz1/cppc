#pragma once
#include <string>

struct Type {
  enum class Base { VOID, INT, DOUBLE, CHAR, ARRAY, BOOL, FUNCTION } base;
  int arraySize;
  bool isArray;
  bool operator==(const Type& rhs) const {
    return base == rhs.base && arraySize == rhs.arraySize &&
           isArray == rhs.isArray;
  }
};
