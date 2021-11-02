#pragma once

struct Type {
  enum class Base { VOID, INT, DOUBLE, CHAR, ARRAY, BOOL, FUNCTION } base;
  int arraySize;
  bool isArray;
};
