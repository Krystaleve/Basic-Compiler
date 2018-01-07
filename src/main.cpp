#include <iostream>
#include "ast/ast.h"
#include "ast/gen-ctx.h"

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

    YacCodeGenContext context;
    root->generate(context);
    context.print_code();
    return 0;
}