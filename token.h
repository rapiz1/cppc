#ifndef __CLOX_TOKEN__
#define __CLOX_TOKEN__
#include <iostream>
#include <string>
enum TokenType {
  // Not a valid token
  INVALID,

  // Single-character tokens.
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,
  LEFT_SQUARE,
  RIGHT_SQUARE,

  // One or two character tokens.
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,
  PLUSPLUS,
  MINUSMINUS,
  RIGHT_ARROW,

  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,
  CHARACTER,

  // Keywords.
  AND,
  CLASS,
  ELSE,
  FALSE,
  FUNCTION,
  FOR,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  BREAK,
  SUPER,
  THIS,
  TRUE,
  VAR,
  INT,
  DOUBLE,
  FLOAT,
  CHAR,
  BOOL,
  VOID,
  WHILE,
  ASSERT,

  TEOF
};
struct Token {
  TokenType tokenType;
  std::string lexeme;
  // ??? literal
  int line;

  Token(TokenType tokenType = INVALID, std::string lexeme = "invalid",
        int line = -1)
      : tokenType(tokenType), lexeme(lexeme), line(line) {}
};
std::ostream& operator<<(std::ostream& out, Token token);

TokenType string2keyword(const std::string& s);
#endif
