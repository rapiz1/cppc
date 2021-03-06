#include "object.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

void object(CodeGenVisitor v) {
  auto TargetTriple = sys::getDefaultTargetTriple();
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();
  std::string Error;

  auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

  // Print an error and exit if we couldn't find the requested target.
  // This generally occurs if we've forgotten to initialise the
  // TargetRegistry or we have a bogus target triple.
  if (!Target) {
    errs() << Error;
    return;
  }

  auto CPU = "generic";
  auto Features = "";

  TargetOptions opt;
  auto RM = Optional<Reloc::Model>();
  auto TargetMachine =
      Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
  v.getMod()->setDataLayout(TargetMachine->createDataLayout());
  v.getMod()->setTargetTriple(TargetTriple);

  auto Filename = "output.o";
  std::error_code EC;
  raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);

  if (EC) {
    errs() << "Could not open file: " << EC.message();
    return;
  }

  legacy::PassManager pass;
  auto FileType = CGFT_ObjectFile;

  if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
    errs() << "TargetMachine can't emit a file of this type";
    return;
  }

  pass.run(*v.getMod());
  dest.flush();

  errs() << "Object file generated: " << Filename << "\n";
}

void link(const std::string prog = "a.out") {
  // generated by clang output.o -v
  static const std::string ldCmd =
      "/usr/bin/ld -pie --eh-frame-hdr -m elf_x86_64 -dynamic-linker "
      "/lib64/ld-linux-x86-64.so.2 "
      "/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/11.1.0/../../../../lib64/"
      "Scrt1.o "
      "/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/11.1.0/../../../../lib64/"
      "crti.o /usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/11.1.0/crtbeginS.o "
      "-L/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/11.1.0 "
      "-L/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/11.1.0/../../../../lib64 "
      "-L/usr/bin/../lib64 -L/lib/../lib64 -L/usr/lib/../lib64 "
      "-L/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/11.1.0/../../.. "
      "-L/usr/bin/../lib -L/lib -L/usr/lib output.o -lgcc --as-needed -lgcc_s "
      "--no-as-needed -lc -lgcc --as-needed -lgcc_s --no-as-needed "
      "/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/11.1.0/crtendS.o "
      "/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/11.1.0/../../../../lib64/"
      "crtn.o -o " +
      prog;
  if (system(ldCmd.c_str()) == 0) errs() << "Linked: " << prog << "\n";
}

void compile(CodeGenVisitor v) {
  object(v);
  link();
}
