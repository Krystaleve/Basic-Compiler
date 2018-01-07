#ifndef STATEMENT_H_INCLUDE
#define STATEMENT_H_INCLUDE

#include "ast.h"

class YacReturnStatement: public YacSyntaxTreeNode {
public:
    YacSyntaxTreeNode *expression;

    explicit YacReturnStatement(YacSyntaxTreeNode *expression = nullptr);
    llvm::Value* generate(YacCodeGenContext &context) override;
};


#endif