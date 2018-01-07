#include <iostream>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include "ast/ast.h"
#include "ast/context.h"

using namespace std;
using namespace llvm;

extern FILE* yyin;
extern int yyparse();

YacSyntaxTreeNodeList *root;

int main(int argc, char **argv)
{
    yyin = (argc > 1) ? fopen(argv[1], "r") : stdin;
    if (yyin == nullptr) {
        cerr << "Cannot open file." << std::endl;
        return 1;
    }
    if (yyparse() && !root)
        return 1;

    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    YacCodeGenContext context;
    root->generate(context);
    context.print();
    return context.execute();
}