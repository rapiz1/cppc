#include <fstream>
#include <iostream>
#include <string>

#include "cmdargs.h"
#include "parser.h"
#include "scanner.h"

using namespace std;

CmdArgs* options;

ExecContext context;  // Let the repl use a single global context in one run
int run(const string& s) {
  Scanner scanner(s);
  auto tokens = scanner.scanTokens();
  if (options->getDebug())
    for (auto t : tokens) cerr << t << endl;
  Parser parser;
  auto stmts = parser.parse(tokens);
  ExecVisitor v(context);
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
  options = new CmdArgs(argc, argv);
  if (options->getMode() == RunMode::FROM_FILE) {
    return runFile(string(argv[1]));
  } else {
    return runPrompt();
  }
}
