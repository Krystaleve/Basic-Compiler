#ifndef AST_H_INCLUDE
#define AST_H_INCLUDE

#include <llvm/IR/Value.h>
#include <vector>

class YacCodeGenContext;

class YacSyntaxTreeNode {
public:
    virtual ~YacSyntaxTreeNode() = default;
    virtual llvm::Value* generate(YacCodeGenContext &context) = 0;
};

class YacSyntaxEmptyNode: public YacSyntaxTreeNode {
public:
    llvm::Value* generate(YacCodeGenContext &context) override {
        return nullptr;
    }
};

class YacSyntaxTreeNodeList: public YacSyntaxTreeNode {
public:
    std::vector<YacSyntaxTreeNode *> children;
    llvm::Value* generate(YacCodeGenContext &context) override;
};

#endif