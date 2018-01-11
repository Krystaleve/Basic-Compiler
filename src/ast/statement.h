#ifndef STATEMENT_H_INCLUDE
#define STATEMENT_H_INCLUDE

#include "ast.h"

class YacExpression;

class YacReturnStatement: public YacSyntaxTreeNode {
public:
    YacExpression *expression;

    explicit YacReturnStatement(YacExpression *expression = nullptr);
    llvm::Value* generate(YacSemanticAnalyzer &context) override;
};

class YacIfStatement: public YacSyntaxTreeNode {
    YacExpression *expression;
    YacSyntaxTreeNode *if_clause, *else_clause;

    explicit YacIfStatement(YacExpression *expression = nullptr, YacSyntaxTreeNode *if_clause = nullptr, YacSyntaxTreeNode *else_clause = nullptr);
    llvm::Value* generate(YacSemanticAnalyzer &context) override;
};

class YacBreakableStatement: virtual public YacSyntaxTreeNode {
    virtual llvm::Value generateBreak() = 0;
};

class YacContinueableStatement: virtual public YacSyntaxTreeNode {
    virtual llvm::Value generateContinue() = 0;
};

class YacForStatement: public YacBreakableStatement, public YacContinueableStatement {
    YacExpression *expression1, *expression2, *expression3;
    YacSyntaxTreeNode *body;

    explicit YacForStatement(YacExpression *expression1 = nullptr, YacExpression *expression2 = nullptr,
                             YacExpression *expression3 = nullptr, YacSyntaxTreeNode *body = nullptr);
    llvm::Value* generate(YacSemanticAnalyzer &context) override;
    llvm::Value generateBreak() override;
    llvm::Value generateContinue() override;
};

class YacWhileStatment: public YacBreakableStatement, public YacContinueableStatement {
    YacExpression *expression;
    YacSyntaxTreeNode *body;

    explicit YacWhileStatment(YacExpression *expression = nullptr, YacSyntaxTreeNode *body = nullptr);
    llvm::Value* generate(YacSemanticAnalyzer &context) override;
    llvm::Value generateBreak() override;
    llvm::Value generateContinue() override;
};

class YacDoWhileStatment: public YacBreakableStatement, public YacContinueableStatement {
    YacExpression *expression;
    YacSyntaxTreeNode *body;

    explicit YacDoWhileStatment(YacExpression *expression = nullptr, YacSyntaxTreeNode *body = nullptr);
    llvm::Value* generate(YacSemanticAnalyzer &context) override;
    llvm::Value generateBreak() override;
    llvm::Value generateContinue() override;
};

#endif