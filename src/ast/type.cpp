#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
#include <iostream>
#include "type.h"

bool isValidVariableType(llvm::Type *type)
{
    if (type->isArrayTy()) {
        auto array_type = llvm::cast<llvm::ArrayType>(type);
        if (array_type->getNumElements() == 0) {
            std::cerr << "array length must be greater than zero" << std::endl;
            return false;
        }
        return isValidVariableType(array_type->getElementType());
    } else if (type->isPointerTy()) {
        auto element_type = llvm::cast<llvm::PointerType>(type)->getElementType();
        if (element_type->isVoidTy())
            return true;
        if (element_type->isFunctionTy())
            return isValidFunctionType(llvm::cast<llvm::FunctionType>(element_type));
        return isValidVariableType(element_type);
    }
    if (!type->isFirstClassType()) {
        std::cerr << "variable has incomplete type" << std::endl;
        return false;
    }
    return true;
}

bool isValidFunctionType(llvm::FunctionType *type)
{
    auto return_type = type->getReturnType();
    if (return_type->isArrayTy()) {
        std::cerr << "function cannot return array type" << std::endl;
        return false;
    }
    if (return_type->isFunctionTy()) {
        std::cerr << "function cannot return function type" << std::endl;
        return false;
    }
    if (!return_type->isVoidTy() && !isValidVariableType(return_type))
        return false;
    for (auto arg_type: type->params())
        if (!isValidVariableType(arg_type))
            return false;
    return true;
}

llvm::Type *castToParameterType(llvm::Type *type)
{
    if (type->isArrayTy())
        return llvm::PointerType::getUnqual(llvm::cast<llvm::ArrayType>(type)->getElementType());
    if (type->isFunctionTy())
        return llvm::PointerType::getUnqual(type);
    return type;
}

llvm::Value *castToType(llvm::Value *value, llvm::Type *type, YacCodeGenContext &context)
{
    if (value->getType() == type)
        return value;
    auto code = llvm::CastInst::getCastOpcode(value, true, type, true);
    return llvm::CastInst::Create(code, value, type, "", context.block());
}
