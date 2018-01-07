#ifndef EXPRESSION_H_INCLUDE
#define EXPRESSION_H_INCLUDE

#include "ast.h"

class YacPrimaryExpression: public YacSyntaxTreeNode {
public:
    llvm::Value *value;
    explicit YacPrimaryExpression(llvm::Value *value);
    llvm::Value* generate(YacCodeGenContext &context) override;
};

#endif