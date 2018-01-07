#ifndef DECLARATOR_H_INCLUDE
#define DECLARATOR_H_INCLUDE

#include <llvm/IR/Type.h>
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
};

typedef std::vector<YacDeclaratorBuilder *> YacDeclaratorBuilderList;

class YacDeclaration: public YacSyntaxTreeNode {
public:
    llvm::Type *type;
    std::string *identifier;

    YacDeclaration(llvm::Type *type, std::string *identifier);
    llvm::Value* generate(YacCodeGenContext &context) override;
};

#endif
