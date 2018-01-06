#ifndef EXPRESSION_H_INCLUDE
#define EXPRESSION_H_INCLUDE

#include "ast.h"

class YacExpression: public YacSyntaxTreeNode {
};

class YacChar: public YacExpression {
public:
    explicit YacChar(char value): value(value) {}
    llvm::Value* generate(YacCodeGenContext &context) override;
private:
    char value;
};

class YacBinaryOperator: public YacExpression {
public:
};

#endif