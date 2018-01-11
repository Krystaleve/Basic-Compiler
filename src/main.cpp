#include <iostream>
#include <cstring>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/FileSystem.h>
#include "ast/ast.h"
#include "ast/context.h"
#include "ast/expression.h"
#include "ast/declaration.h"

using namespace std;
using namespace llvm;

extern FILE* yyin;
extern int yyparse();

const char *default_args[] = {"main"};

int main(int argc, const char **argv) try {
    bool compile = false, jit = false;
    const char *output = nullptr;
    int i;
    for (i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (strcmp(arg, "-c") == 0 || strcmp(arg, "--compile") == 0)
            compile = true;
        else if (strcmp(arg, "-e") == 0 || strcmp(arg, "--execute") == 0)
            jit = true;
        else if (strcmp(arg, "-o") == 0 || strcmp(arg, "--output") == 0) {
            if (++i < argc)
                output = argv[i];
            else {
                cerr << "yac: missing output file" << std::endl;
                return 1;
            }
        } else
            break;
    }
    if (output != nullptr)
        compile = true;
    if (!compile)
        jit = true;
    if (i < argc) {
        yyin = fopen(argv[i], "r");
        if (yyin == nullptr) {
            cerr << "yac: cannot open input file" << std::endl;
            return 1;
        }
        input = argv[i];
    } else
        yyin = stdin;

    if (yyparse() && !root)
        return 1;

    YacSemanticAnalyzer context;
    root->generate(context);
    if (compile) {
        if (output) {
            std::error_code err;
            raw_fd_ostream out(output, err, sys::fs::F_Text);
            if (err) {
                cerr << "yac: cannot open output file" << std::endl;
                return 1;
            }
            context.print(out);
        } else
            context.print(outs());
    }

    if (jit) {
        llvm::Value *func = context.find(findInScopes("main"));
        if (!func) {
            cerr << "yac: cannot find main" << std::endl;
            return 1;
        }
        if (!isa<llvm::Function>(func)) {
            cerr << "yac: main is non-function" << std::endl;
            return 1;
        }
        InitializeNativeTarget();
        InitializeNativeTargetAsmPrinter();
        InitializeNativeTargetAsmParser();
        if (i < argc)
            return context.execute(llvm::cast<llvm::Function>(func), argc - i, argv);
        return context.execute(llvm::cast<llvm::Function>(func), sizeof(default_args) / sizeof(default_args[0]), default_args);
    }
    return 0;
} catch (YacSemanticError &err) {
    cerr << "yac: " << err << std::endl;
    return 1;
} catch (YacSyntaxError &err) {
    cerr << "yac: " << err << std::endl;
    return 1;
}