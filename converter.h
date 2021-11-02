#pragma once
#include <cassert>

#include "ast.h"
#include "type.h"

class Converter {
 public:
  static Double* converToDouble(Literal* l) {
    Double* d = dynamic_cast<Double*>(l);
    if (d) return d;

    Integer* i = dynamic_cast<Integer*>(l);
    if (i) return new Double(i->value);
    return nullptr;
  };
  static Integer* convertToInteger(Literal* l) {
    Integer* i = dynamic_cast<Integer*>(l);
    if (i) return i;

    Double* d = dynamic_cast<Double*>(l);
    if (d)
      return new Integer((int)d->value);
    else {
      Boolean* b = dynamic_cast<Boolean*>(l);
      if (b) return new Integer((int)b->value);
    }
    return nullptr;
  };
  static bool equal(Literal* x, Literal* y) {
    auto i1 = dynamic_cast<Integer*>(x);
    auto i2 = dynamic_cast<Integer*>(y);
    if (i1 && i2) return true;
    auto d1 = dynamic_cast<Double*>(x);
    auto d2 = dynamic_cast<Double*>(y);
    if (d1 && d2) return true;
    auto b1 = dynamic_cast<Boolean*>(x);
    auto b2 = dynamic_cast<Boolean*>(y);
    if (b1 && b2) return true;
    return false;
  }
};
