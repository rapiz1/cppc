# cppc

C compiler in C++

## Background
This is a learning project for [Crafting interpreters](https://craftinginterpreters.com/).


Most part of C is supported. Support for pointer is hacky.

For which part is supported, see test code under `tests/`.

## Dependencies

Graphviz binary and llvm must be installed. llvm headers are required.
Linker commands are dependent on platform and distribution. See the link function in `object.cc`.

## Usage

### Compile

```
make
```

### Test

```
make test
```

### Run

```
./clox SOURCE
```
And then `./a.out`, `./output.o`, `./output.dot` and `./output.png` is genereated.


`./a.out` is the executable file.

`./output.o` is the object file.

`./output.dot` is the dot description of the AST.

`./output.png` is the graph generated according to `./output.dot`

## Leftover

Generally speaking, type system is a whole mess. Basic types (int, double, char, bool) works, and arrays should work in most case.

Other things are like:

- Pointer is not supported.
- Pointer arguments are supported but in a hacky way.
- Array is partly supported.
- Struct and unions are not supported.
