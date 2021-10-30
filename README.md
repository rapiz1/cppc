# clox

Lox(well, not really) Interpreter in C++ (under development)

## Background
This is a learning project for [Crafting interpreters](https://craftinginterpreters.com/).

I'm not actully implementing the authentic Lox. I adopted it according to my likes.

Lox is a language that is very similar to C, but with dynamic types. Let's see an example.

```
// calculate the greatest common divisor
function gcd(a, b) {
  if (b == 0) return a;
  while (a >= b) a = a - b;
  return gcd(b, a);
}

print gcd(99, 121);
assert gcd(99, 121) == 11;
```

## Roadmap

- [x] basic lexer
- [x] basic parser
- [x] ``+,-,*,/,=,==,!=,<,>,<=,>=,(,)``
- [ ] ``+=, -=, *=, /=``
- [x] support ++, --. but no `----a` bullshit! theses ops return rvalue
- [x] print statement
- [x] expr statement
- [x] var declartion
- [x] block scoping
- [x] for, while block
- [x] branch
- [x] assert statement
- [x] **testing the language with itself (built-in assert)!** see `./tests`
- [ ] no continue! i'm lazy
- [x] break
- [x] function
- [x] return (can run gcd now!)
- [ ] error recover
- [ ] no local function or closure! i don't like it
