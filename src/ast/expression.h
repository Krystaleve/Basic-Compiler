#ifndef EXPRESSION_H_INCLUDE
#define EXPRESSION_H_INCLUDE

#include "ast.h"

class YacExpression: public YacSyntaxTreeNode {
public:
    // can return anything (value will not be used)
    // return nullptr on error or on no side effect expression
    llvm::Value* generate(YacSemanticAnalyzer &context) override {
        return nullptr;
    }
    // can return pointer to integer, float, pointer (aka pointer to first class)
    // return nullptr on error
    virtual llvm::Value *generateLvalue(YacSemanticAnalyzer &context) {
        std::cerr << YacSemanticError("expression is not lvalue", this) << std::endl;
        return nullptr;
    }
    // can return integer, float, pointer (aka first class)
    // return nullptr on error
    virtual llvm::Value *generateRvalue(YacSemanticAnalyzer &context) {
        std::cerr << YacSemanticError("expression is not lvalue", this) << std::endl;
        return nullptr;
    }
};

class YacEmptyExpression: public YacExpression {
    llvm::Value *generateLvalue(YacSemanticAnalyzer &context) override {
        return nullptr;
    }
    llvm::Value *generateRvalue(YacSemanticAnalyzer &context) override {
        return nullptr;
    }
};

class YacLvalueExpression: public YacExpression {
    llvm::Value *generate(YacSemanticAnalyzer &context) override {
        return generateLvalue(context);
    }
    llvm::Value *generateRvalue(YacSemanticAnalyzer &context) override;
};

class YacRvalueExpression: public YacExpression {
    llvm::Value *generate(YacSemanticAnalyzer &context) override {
        return generateRvalue(context);
    }
};


class YacConstantExpression: public YacExpression {
public:
    llvm::Value *value;
    explicit YacConstantExpression(llvm::Value *value);
    bool isString() {
        return value->getType()->isArrayTy();
    }
    llvm::Value *generateRvalue(YacSemanticAnalyzer &context) override;
};

typedef std::vector<YacExpression *> YacExpressionList;

class YacCallExpression: public YacExpression {
public:
    YacExpression *func;
    YacExpressionList *args;
    explicit YacCallExpression(YacExpression *func, YacExpressionList *args = nullptr);
    llvm::FunctionType *getFunc(llvm::Value *function, YacSemanticAnalyzer &context);
    llvm::Value *doGenerate(llvm::Value *function, llvm::FunctionType *function_type, YacSemanticAnalyzer &context);
    llvm::Value *generate(YacSemanticAnalyzer &context) override;
    llvm::Value *generateRvalue(YacSemanticAnalyzer &context) override;
};

// all binary arithmetic expression and comparision expression
// (aka `rvalue = rvalue operator rvalue')
class YacBinaryExpression: public YacRvalueExpression {
public:
    YacExpression *left, *right;
    int token;
    explicit YacBinaryExpression(YacExpression *left, YacExpression *right, int token);
    llvm::Value *generateRvalue(YacSemanticAnalyzer &context) override;
};

class YacObjectExpression: public YacLvalueExpression {
public:
    YacDeclaration *declaration;
    explicit YacObjectExpression(YacDeclaration *declaration);
    llvm::Value *generateLvalue(YacSemanticAnalyzer &context) override;
};

class YacAssignmentExpression: public YacLvalueExpression {
public:
    YacExpression *left, *right;
    explicit YacAssignmentExpression(YacExpression *left, YacExpression *right);
    llvm::Value *generateLvalue(YacSemanticAnalyzer &context) override;
};

class YacCompoundAssignmentExpression: public YacLvalueExpression {
public:
    YacExpression *left, *right;
    int token;
    explicit YacCompoundAssignmentExpression(YacExpression *left, YacExpression *right, int token);
    llvm::Value *generateLvalue(YacSemanticAnalyzer &context) override;
};

bool binaryExpressionCheck(llvm::Value *left, llvm::Value *right, int token);
llvm::Value *binaryExpression(llvm::Value *left, llvm::Value *right, int token, YacSemanticAnalyzer &context);

#endif