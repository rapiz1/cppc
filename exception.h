#ifndef __H_EXCEPTION__
#define __H_EXCEPTION__
#include <exception>
class RuntimeError : public std::exception {};
class TypeError : public RuntimeError {};
class BindError : public TypeError {};
#endif
