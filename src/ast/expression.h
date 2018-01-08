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
    // can return pointer
    // return nullptr if error
    virtual llvm::Value *generateLvalue(YacCodeGenContext &context) {
        return nullptr;
    }
    // can return integer, float, pointer (aka first class)
    // return nullptr if error
    virtual llvm::Value *generateRvalue(YacCodeGenContext &context) {
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

#endif