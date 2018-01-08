#ifndef STATEMENT_H_INCLUDE
#define STATEMENT_H_INCLUDE

#include "ast.h"
#include "expression.h"

class YacReturnStatement: public YacSyntaxTreeNode {
public:
    YacExpression *expression;

    explicit YacReturnStatement(YacExpression *expression = nullptr);
    llvm::Value* generate(YacCodeGenContext &context) override;
};


#endif