#include <iostream>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/GlobalVariable.h>
#include "declarator.h"
#include "gen-ctx.h"

YacDeclaratorIdentifier::YacDeclaratorIdentifier(std::string *identifier)
    : m_identifier(identifier) {}

std::string *YacDeclaratorIdentifier::identifier() {
    return m_identifier;
}


YacDeclaratorHasParent::YacDeclaratorHasParent(YacDeclaratorBuilder *parent)
        : parent(parent) {}

llvm::Type *YacDeclaratorHasParent::type(llvm::Type *specifier) {
    return parent->type(specifier);
}

std::string *YacDeclaratorHasParent::identifier() {
    return parent->identifier();
}


YacDeclaratorPointer::YacDeclaratorPointer(YacDeclaratorBuilder *parent)
    : YacDeclaratorHasParent(parent)  {}

llvm::Type *YacDeclaratorPointer::type(llvm::Type *specifier) {
    return YacDeclaratorHasParent::type(llvm::PointerType::getUnqual(specifier));
}

YacDeclaratorArray::YacDeclaratorArray(YacDeclaratorBuilder *parent, uint64_t num)
    : YacDeclaratorHasParent(parent), m_num(num) {}

llvm::Type *YacDeclaratorArray::type(llvm::Type *specifier) {
    return YacDeclaratorHasParent::type(llvm::ArrayType::get(specifier, m_num));
}

YacDeclaration::YacDeclaration(llvm::Type *type, std::string *identifier)
    : type(type), identifier(identifier) {}

llvm::Value *YacDeclaration::generate(YacCodeGenContext &context)
{
    if (identifier == nullptr)
        return nullptr;
    if (context.is_top_level()) {
        if (context.globals().find(*identifier) != context.globals().end()) {
            std::cerr << "Global variable " << *identifier << "\" already exists" << std::endl;
            return nullptr;
        }
        llvm::Constant *init = llvm::Constant::getNullValue(type);
        llvm::Value *var = new llvm::GlobalVariable(context.module(), type, false, llvm::GlobalVariable::CommonLinkage,
                                                    init, *identifier);
        context.globals().insert(std::make_pair(*identifier, var));
    }
    return nullptr;
}
