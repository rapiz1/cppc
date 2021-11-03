#include "scanner.h"

std::string Scanner::get_lexeme(TokenType type) {
  if (type == STRING)
    return source.substr(start + 1, current - start - 2);
  else if (type == CHAR)
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

void Scanner::readChar() {
  while (peek() != '\'') {
    if (peek() == '\n') line++;
    advance();
  }
  advance();
  std::string lexeme = get_lexeme(CHAR);
  int len = lexeme.size();
  if (len == 1)
    ;
  else if (len == 2) {
    if (source[start + 1] == '\\') {
      char c = source[start + 2];
      char cc = 0;
      if (c == 'n')
        cc = '\n';
      else if (c == 't')
        cc = '\t';
      else if (c == '0')
        cc = '\0';
      else
        error("unimplemented escape sequence");
      lexeme = std::string(1, cc);
    } else
      error("bad escape sequence");
  } else
    error("bad char literal " + lexeme);
  tokens.push_back(Token(CHARACTER, lexeme, line));
}

void Scanner::number() {
  while (isdigit(peek())) advance();
  if (peek() == '.') {
    advance();
    if (isdigit(peek())) {
      while (isdigit(peek())) advance();
    } else {
      error("broken number expression when lexing");
    }
  }
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
      addToken(match('=') ? STAR_EQUAL : STAR);
      break;
    case '+':
      addToken(match('+') ? PLUSPLUS : match('=') ? PLUS_EQUAL : PLUS);
      break;
    case '-':
      addToken(match('-')   ? MINUSMINUS
               : match('>') ? RIGHT_ARROW
               : match('=') ? MINUS_EQUAL
                            : MINUS);
      break;
    case '/':
      if (match('/')) {
        while (peek() != '\n' && !eof()) advance();
      } else {
        addToken(match('=') ? SLASH_EQUAL : SLASH);
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
    case '%':
      addToken(match('=') ? PERCENT_EQUAL : PERCENT);
      break;
    case '(':
      addToken(LEFT_PAREN);
      break;
    case ')':
      addToken(RIGHT_PAREN);
      break;
    case '[':
      addToken(LEFT_SQUARE);
      break;
    case ']':
      addToken(RIGHT_SQUARE);
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
    case '\'':
      readChar();
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
