#ifndef AST_H_INCLUDE
#define AST_H_INCLUDE

#include <llvm/IR/Value.h>

class YacCodeGenContext;

class YacSyntaxTreeNode {
public:
    virtual ~YacSyntaxTreeNode() {}
    virtual llvm::Value* generate(YacCodeGenContext &context) = 0;
};

#endif