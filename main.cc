#include <fstream>
#include <iostream>
#include <string>

#include "cmdargs.h"
#include "object.h"
#include "parser.h"
#include "scanner.h"

using namespace std;

CmdArgs* options;

int run(const string& s) {
  Scanner scanner(s);
  auto tokens = scanner.scanTokens();
  if (options->printLex())
    for (auto t : tokens) cerr << t << endl;
  Parser parser;
  auto stmts = parser.parse(tokens);
  Scope scope;
  llvmWrapper l;
  CodeGenVisitor v(scope, l);
  for (auto s : stmts) {
    v.visit(s);
    cout << endl;
  }
  if (options->printIR()) l.mod->print(llvm::outs(), nullptr);
  compile(v);
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

int main(int argc, char** argv) {
  options = new CmdArgs(argc, argv);
  return runFile(string(argv[1]));
}
