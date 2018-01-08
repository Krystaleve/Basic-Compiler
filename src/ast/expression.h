#ifndef EXPRESSION_H_INCLUDE
#define EXPRESSION_H_INCLUDE

#include "ast.h"

class YacExpression: public YacSyntaxTreeNode {
public:
    // can return anything (value will not be used)
    // return nullptr on error or on no side effect expression
    llvm::Value* generate(YacCodeGenContext &context) override {
        return nullptr;
    }
    // can return pointer to integer, float, pointer (aka pointer to first class)
    // return nullptr on error
    virtual llvm::Value *generateLvalue(YacCodeGenContext &context) {
        std::cerr << "expression is not lvalue" << std::endl;
        return nullptr;
    }
    // can return integer, float, pointer (aka first class)
    // return nullptr on error
    virtual llvm::Value *generateRvalue(YacCodeGenContext &context) {
        std::cerr << "expression is not rvalue" << std::endl;
        return nullptr;
    }
};

class YacLvalueExpression: public YacExpression {
    llvm::Value *generateRvalue(YacCodeGenContext &context) override;
};

class YacConstantExpression: public YacExpression {
public:
    llvm::Value *value;
    explicit YacConstantExpression(llvm::Value *value);
    llvm::Value *generateRvalue(YacCodeGenContext &context) override;
};

class YacIdentifierExpression: public YacLvalueExpression {
public:
    std::string *identifier;
    explicit YacIdentifierExpression(std::string *identifier);
    llvm::Value *find(YacCodeGenContext &context);
    llvm::Value *generate(YacCodeGenContext &context) override;
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

// all binary arithmetic expression and comparision expression
// (aka `rvalue = rvalue operator rvalue' without short-circuit evaluation)
class YacBinaryExpression: public YacExpression {
public:
    YacExpression *left, *right;
    int token;
    explicit YacBinaryExpression(YacExpression *left, YacExpression *right, int token);
    llvm::Value *generate(YacCodeGenContext &context) override;
    llvm::Value *generateRvalue(YacCodeGenContext &context) override;
};

class YacBinaryLogicExpression: public YacExpression {
public:
    YacExpression *left, *right;
    int token;
    explicit YacBinaryLogicExpression(YacExpression *left, YacExpression *right, int token);
    llvm::Value *generate(YacCodeGenContext &context) override;
    llvm::Value *generateRvalue(YacCodeGenContext &context) override;
};

class YacAssignmentExpression: public YacLvalueExpression {
public:
    YacExpression *left, *right;
    explicit YacAssignmentExpression(YacExpression *left, YacExpression *right);
    llvm::Value *generate(YacCodeGenContext &context) override;
    llvm::Value *generateLvalue(YacCodeGenContext &context) override;
};

class YacCompoundAssignmentExpression: public YacLvalueExpression {
public:
    YacExpression *left, *right;
    int token;
    explicit YacCompoundAssignmentExpression(YacExpression *left, YacExpression *right, int token);
    llvm::Value *generate(YacCodeGenContext &context) override;
    llvm::Value *generateLvalue(YacCodeGenContext &context) override;
};

bool binaryExpressionCheck(llvm::Value *left, llvm::Value *right, int token);
llvm::Value *binaryExpression(llvm::Value *left, llvm::Value *right, int token, YacCodeGenContext &context);

#endif