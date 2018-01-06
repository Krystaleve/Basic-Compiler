#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include "declarator.h"

YacDeclaratorIdentifier::YacDeclaratorIdentifier(std::string *identifier)
    : m_identifier(identifier) {}

std::string *YacDeclaratorIdentifier::identifier() {
    return m_identifier;
}


YacDeclaratorHasParent::YacDeclaratorHasParent(YacDeclaratorBuilder *parent)
        : parent(parent) {}

std::string *YacDeclaratorHasParent::identifier() {
    return parent->identifier();
}


YacDeclaratorPointer::YacDeclaratorPointer(YacDeclaratorBuilder *parent)
    : YacDeclaratorHasParent(parent)  {}

llvm::Type *YacDeclaratorPointer::type(llvm::Type *specifier) {
    return llvm::PointerType::getUnqual(YacDeclaratorBuilder::type(specifier));
}

YacDeclaratorArray::YacDeclaratorArray(YacDeclaratorBuilder *parent, uint64_t num)
    : YacDeclaratorHasParent(parent), m_num(num) {}

llvm::Type *YacDeclaratorArray::type(llvm::Type *specifier) {
    return llvm::ArrayType::get(YacDeclaratorBuilder::type(specifier), m_num);
}

YacDeclaration::YacDeclaration(llvm::Type *type, std::string *m_identifier)
    : type(type), identifier(m_identifier) {}

#include <iostream>

llvm::Value *YacDeclaration::generate(YacCodeGenContext &context)
{
    std::cout << "hello" << std::endl;
    return nullptr;
}
