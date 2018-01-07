#ifndef DECLARATOR_H_INCLUDE
#define DECLARATOR_H_INCLUDE

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <string>
#include <vector>

#include "ast.h"

class YacDeclaratorBuilder {
public:
    virtual ~YacDeclaratorBuilder() = default;
    virtual llvm::Type *type(llvm::Type *specifier) {
        return specifier;
    }
    virtual std::string *identifier() = 0;
};

class YacDeclaratorIdentifier: public YacDeclaratorBuilder {
public:
    explicit YacDeclaratorIdentifier(std::string *identifier = nullptr);
    std::string *identifier() override;
private:
    std::string *m_identifier;
};

class YacDeclaratorHasParent: public YacDeclaratorBuilder {
public:
    explicit YacDeclaratorHasParent(YacDeclaratorBuilder *parent);
    llvm::Type *type(llvm::Type *specifier) override;
    std::string *identifier() override;
private:
    YacDeclaratorBuilder *parent;
};

class YacDeclaratorPointer: public YacDeclaratorHasParent {
public:
    explicit YacDeclaratorPointer(YacDeclaratorBuilder *parent);
    llvm::Type *type(llvm::Type *specifier) override;
};

class YacDeclaratorArray: public YacDeclaratorHasParent {
public:
    YacDeclaratorArray(YacDeclaratorBuilder *parent, uint64_t num);
    llvm::Type *type(llvm::Type *specifier) override;
private:
    uint64_t m_num;
};

class YacDeclaratorFunction: public YacDeclaratorHasParent {
public:
    explicit YacDeclaratorFunction(YacDeclaratorBuilder *parent, YacSyntaxTreeNode *node = nullptr, bool var_arg = false);
    llvm::Type *type(llvm::Type *specifier) override;
    YacSyntaxTreeNode *node() {
        return m_node;
    }
private:
    YacSyntaxTreeNode *m_node;
    bool m_var_arg;
};

typedef std::vector<YacDeclaratorBuilder *> YacDeclaratorBuilderList;

class YacDeclaration: public YacSyntaxTreeNode {
public:
    llvm::Type *type;
    std::string *identifier;

    explicit YacDeclaration(llvm::Type *type, std::string *identifier = nullptr);
    llvm::Value* generate(YacCodeGenContext &context) override;
};

class YacFunctionDefinition: public YacSyntaxTreeNode {
public:
    llvm::FunctionType *type;
    std::string *identifier;
    std::vector<YacDeclaration *> params;
    YacSyntaxTreeNode *body;

    explicit YacFunctionDefinition(llvm::FunctionType *type, std::string *identifier, std::vector<YacDeclaration *> &&params, YacSyntaxTreeNode *body);
    llvm::Value* generate(YacCodeGenContext &context) override;
};

#endif
