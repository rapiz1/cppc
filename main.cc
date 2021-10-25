#include <fstream>
#include <iostream>
#include <string>

#include "parser.h"
#include "scanner.h"

using namespace std;

const string usage = "Usage: clox [script]\n";

int run(const string& s) {
  Scanner scanner(s);
  auto tokens = scanner.scanTokens();
  for (auto t : tokens) cout << t << endl;
  Parser parser;
  auto stmts = parser.parse(tokens);
  ExecVisitor v;
  for (auto s : stmts) {
    v.visit(s);
  }
  return 0;
}

int runFile(const string& file) {
  ifstream fin(file);
  if (!fin.good()) {
    cerr << "Cannot open script " << file << endl;
    return -1;
  }
  string file_content, s;
  while (getline(fin, s)) {
    file_content += s;
    file_content += "\n";
  }
  return run(file_content);
}

void showPrompt() { cout << ">> "; }

int runPrompt() {
  string line;
  while (showPrompt(), getline(cin, line)) {
    run(line);
  }
  return 0;
}

int main(int argc, char** argv) {
  if (argc > 2) {
    cerr << usage;
    return -1;
  } else if (argc == 2) {
    return runFile(string(argv[1]));
  } else {
    return runPrompt();
  }
}
