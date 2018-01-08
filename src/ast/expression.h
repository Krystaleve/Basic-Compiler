#ifndef EXPRESSION_H_INCLUDE
#define EXPRESSION_H_INCLUDE

#include "ast.h"

class YacExpression: public YacSyntaxTreeNode {
public:
    // can return anything (value will not be used)
    // return nullptr if error
    llvm::Value* generate(YacCodeGenContext &context) override {
        return generateRvalue(context);
    }
    // can return pointer to integer, float, pointer (aka pointer to first class)
    // return nullptr if error
    virtual llvm::Value *generateLvalue(YacCodeGenContext &context) {
        std::cerr << "expression is not lvalue" << std::endl;
        return nullptr;
    }
    // can return integer, float, pointer (aka first class)
    // return nullptr if error
    virtual llvm::Value *generateRvalue(YacCodeGenContext &context) {
        std::cerr << "expression is not rvalue" << std::endl;
        return nullptr;
    }
};

class YacConstantExpression: public YacExpression {
public:
    llvm::Value *value;
    explicit YacConstantExpression(llvm::Value *value);
    llvm::Value *generateRvalue(YacCodeGenContext &context) override;
};

class YacIdentifierExpression: public YacExpression {
public:
    std::string *identifier;
    explicit YacIdentifierExpression(std::string *identifier);
    llvm::Value *find(YacCodeGenContext &context);
    llvm::Value *generateLvalue(YacCodeGenContext &context) override;
    llvm::Value *generateRvalue(YacCodeGenContext &context) override;
};

typedef std::vector<YacExpression *> YacExpressionList;

class YacCallExpression: public YacExpression {
public:
    YacExpression *func;
    YacExpressionList *args;
    explicit YacCallExpression(YacExpression *func, YacExpressionList *args = nullptr);
    llvm::FunctionType *getFunc(llvm::Value *function, YacCodeGenContext &context);
    llvm::Value *doGenerate(llvm::Value *function, llvm::FunctionType *function_type, YacCodeGenContext &context);
    llvm::Value *generate(YacCodeGenContext &context) override;
    llvm::Value *generateRvalue(YacCodeGenContext &context) override;
};

class YacAssignmentExpression: public YacExpression {
public:
    YacExpression *left, *right;
    explicit YacAssignmentExpression(YacExpression *left, YacExpression *right);
    llvm::Value *generateLvalue(YacCodeGenContext &context) override;
    llvm::Value *generateRvalue(YacCodeGenContext &context) override;
};

#endif