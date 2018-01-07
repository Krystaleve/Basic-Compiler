#ifndef EXPRESSION_H_INCLUDE
#define EXPRESSION_H_INCLUDE

#include "ast.h"

class YacExpression: public YacSyntaxTreeNode {
public:
    llvm::Value* generate(YacCodeGenContext &context) override {
        return generateRvalue(context);
    }
    virtual llvm::Value* generateLvalue(YacCodeGenContext &context) {
        return nullptr;
    }
    virtual llvm::Value* generateRvalue(YacCodeGenContext &context) {
        return nullptr;
    }
};

class YacPrimaryExpression: public YacExpression {
public:
    llvm::Value *value;
    explicit YacPrimaryExpression(llvm::Value *value);
    llvm::Value* generateRvalue(YacCodeGenContext &context) override;
};

#endif