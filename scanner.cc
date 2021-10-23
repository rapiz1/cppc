#include "scanner.h"

std::string Scanner::get_lexeme(TokenType type) {
  if (type == STRING)
    return source.substr(start + 1, current - start - 2);
  else
    return source.substr(start, current - start);
}

void Scanner::addToken(TokenType type) {
  auto t = Token(type, get_lexeme(type), line);
  tokens.push_back(t);
}

void Scanner::error(std::string message) {
  errorMessage = message;
  badbit = true;
  std::cerr << "line " << line << ": " << errorMessage << std::endl;
  exit(-1);
}

char Scanner::advance() {
  if (current == source.size()) {
    error("Unexpected EOF");
  }
  return source[current++];
}

char Scanner::peek() { return current >= source.size() ? 0 : source[current]; }

char Scanner::peekNext() {
  return current + 1 >= source.size() ? 0 : source[current + 1];
}

bool Scanner::match(char expected) {
  if (peek() == expected) {
    advance();
    return true;
  }
  return false;
}

void Scanner::string() {
  while (peek() != '"') {
    if (peek() == '\n') line++;
    advance();
  }
  advance();
  addToken(STRING);
}

void Scanner::number() {
  while (isdigit(peek())) advance();
  addToken(NUMBER);
}

void Scanner::identifierOrKeyword() {
  while (isalnum(peek())) advance();
  std::string lexeme = get_lexeme(INVALID);
  TokenType t = string2keyword(lexeme);
  addToken(t == INVALID ? IDENTIFIER : t);
}

void Scanner::scanToken() {
  char c = advance();
  switch (c) {
    case '"':
      string();
      break;
    case '*':
      addToken(STAR);
      break;
    case '+':
      addToken(PLUS);
      break;
    case '-':
      addToken(MINUS);
      break;
    case '/':
      if (match('/')) {
        while (peek() != '\n' && !eof()) advance();
      } else {
        addToken(SLASH);
      }
      break;
    case '!':
      addToken(match('=') ? BANG_EQUAL : BANG);
      break;
    case '=':
      addToken(match('=') ? EQUAL_EQUAL : EQUAL);
      break;
    case '<':
      addToken(match('=') ? LESS_EQUAL : LESS);
      break;
    case '>':
      addToken(match('=') ? GREATER_EQUAL : GREATER);
      break;
    case '(':
      addToken(LEFT_PAREN);
      break;
    case ')':
      addToken(RIGHT_PAREN);
      break;
    case '{':
      addToken(LEFT_BRACE);
      break;
    case '}':
      addToken(RIGHT_BRACE);
      break;
    case ',':
      addToken(COMMA);
      break;
    case '.':
      addToken(DOT);
      break;
    case ';':
      addToken(SEMICOLON);
      break;
    case '\n':
      line++;
    case ' ':
    case '\t':
      break;
    default:
      if (isdigit(c))
        number();
      else if (isalpha(c))
        identifierOrKeyword();
      else {
        error("Unexpected character");
      }
  }
}

std::vector<Token> Scanner::scanTokens() {
  while (!eof()) {
    start = current;
    scanToken();
  }
  return tokens;
}
