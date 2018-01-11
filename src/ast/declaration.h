#ifndef DECLARATION_H_INCLUDE
#define DECLARATION_H_INCLUDE

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>

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

class YacDeclarationList;

class YacDeclaratorFunction: public YacDeclaratorHasParent {
public:
    explicit YacDeclaratorFunction(YacDeclaratorBuilder *parent, YacDeclarationList *list = nullptr, bool var_arg = false);
    llvm::Type *type(llvm::Type *specifier) override;
    YacDeclarationList *node() {
        return m_node;
    }
private:
    YacDeclarationList *m_node;
    bool m_var_arg;
};

typedef std::vector<YacDeclaratorBuilder *> YacDeclaratorBuilderList;

enum YacSpecifiers {
    Typedef  = 1 << 1,
    Auto     = 1 << 2,
    Register = 1 << 3,
    Static   = 1 << 4,
    Extern   = 1 << 5,
};

class YacScope;

class YacDeclaration: public YacSyntaxTreeNode {
public:
    llvm::Type *type;
    std::string *identifier;
    int specifier;

    explicit YacDeclaration(llvm::Type *type, std::string *identifier = nullptr, int specifier = 0);
    llvm::Value* generate(YacSemanticAnalyzer &context) override;
    bool isType() {
        return (specifier & Typedef) != 0;
    }
};


class YacDeclarationList: public YacSyntaxTreeNode {
public:
    std::vector<YacDeclaration *> children;

    void addNode(YacDeclaration *node) {
        if (node)
            children.push_back(node);
    }
    llvm::Value* generate(YacSemanticAnalyzer &context) override {
        for (auto child: children)
            child->generate(context);
        return nullptr;
    };

};

class YacFunctionDefinition: public YacDeclaration {
public:
    YacScope *params;
    YacSyntaxTreeNode *body;

    explicit YacFunctionDefinition(llvm::FunctionType *type, YacScope *params = nullptr, YacSyntaxTreeNode *body = nullptr,
                                   std::string *identifier = nullptr, int specifier = 0);
    llvm::Value* generate(YacSemanticAnalyzer &context) override;
};


#endif
