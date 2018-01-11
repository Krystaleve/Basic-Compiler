#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/Support/raw_ostream.h>
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

llvm::Value *castValueToType(llvm::Value *value, llvm::Type *type, YacSemanticAnalyzer &context)
{
    if (value->getType() == type)
        return value;
    auto code = llvm::CastInst::getCastOpcode(value, true, type, true);
    return llvm::CastInst::Create(code, value, type, "", context.block());
}

llvm::Value *castLvalueToRvalue(llvm::Value *value, YacSemanticAnalyzer &context) {
    if (!value)
        return nullptr;
    assert(value->getType()->isPointerTy());
    auto value_type = value->getType()->getPointerElementType();
    if (value_type->isFunctionTy())
        return value;
    if (value_type->isArrayTy())
        return llvm::GetElementPtrInst::CreateInBounds(value, {
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(YacSemanticAnalyzer::context()), 0),
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(YacSemanticAnalyzer::context()), 0)
        }, "", context.block());
    if (value_type->isVoidTy()) {
        std::cerr << "access void type" << std::endl;
        return nullptr;
    }
    return new llvm::LoadInst(value, "", context.block());
}

std::string getTypeName(llvm::Type *type)
{
    assert(type);
    std::string str;
    llvm::raw_string_ostream stream(str);
    stream << *type;
    return str;
}

bool isObjectType(llvm::Type *type) {
    return !type->isFunctionTy();
}

// http://en.cppreference.com/w/c/language/arithmetic_types
bool isArithmeticType(llvm::Type *type) {
    assert(type);
    return type->isIntegerTy() || type->isFloatingPointTy();
}

// http://en.cppreference.com/w/c/types/NULL
bool isNull(llvm::Value *value) {
    assert(value);
    if (llvm::isa<llvm::ConstantPointerNull>(value)) {
        auto pointer = llvm::cast<llvm::ConstantPointerNull>(value);
        return pointer->getType()->getElementType()->isVoidTy() && pointer->isNullValue();
    }
    return llvm::isa<llvm::ConstantInt>(value) && llvm::cast<llvm::ConstantInt>(value)->isNullValue();
}

// http://en.cppreference.com/w/c/language/type#Compatible_types
bool isCompatible(llvm::Type *left, llvm::Type *right) {
    assert(left && right);
    if (left == right
            || (left->isPointerTy() && right->isPointerTy() && isCompatible(left->getPointerElementType(), right->getPointerElementType()))
            || ((left->isArrayTy() && right->isArrayTy() && isCompatible(left->getArrayElementType(), right->getArrayElementType()))
               && (!left->getArrayNumElements() || !right->getArrayNumElements() || left->getArrayNumElements() == right->getArrayNumElements())))
        return true;
    if (left->isFunctionTy() && right->isFunctionTy()) {
        //auto left_func = llvm::cast<llvm::FunctionType>(left), right_func = llvm::cast<llvm::FunctionType>(right);
        // TODO
    }
    return false;
}


// http://en.cppreference.com/w/c/language/operator_assignment
bool isImplicitlyConvertible(llvm::Value *value, llvm::Type *dst_type) {
    assert(value && dst_type);
    auto src_type = value->getType();
    return (src_type == dst_type
           || (isArithmeticType(src_type) && isArithmeticType(dst_type)))
           || (dst_type->isPointerTy() && ((src_type->isPointerTy() &&
            isCompatible(src_type->getPointerElementType(), dst_type->getPointerElementType())) || isNull(value)));
}

// refer http://en.cppreference.com/w/c/language/conversion#Usual_arithmetic_conversions
void usualArithmeticConversions(llvm::Value *&left, llvm::Value *&right, YacSemanticAnalyzer &context)
{
    assert(left && right);
    auto left_type = left->getType(), right_type = right->getType();
    assert(isArithmeticType(left_type) && isArithmeticType(right_type));
    llvm::Type *result_type;
    if (left_type->isFloatingPointTy() == right_type->isFloatingPointTy()) {
        auto left_size = left_type->getPrimitiveSizeInBits(), right_size = right_type->getPrimitiveSizeInBits();
        assert(left_size && right_size);
        result_type = left_size > right_size ? left_type : right_type;
    } else
        result_type = left_type->isFloatingPointTy() ? left_type : right_type;
    cast(left, result_type, context);
    cast(right, result_type, context);
}

bool isExplicitlyConvertible(llvm::Type *src_type, llvm::Type *dst_type) {
    return false;
}

void cast(llvm::Value *&value, llvm::Type *type, YacSemanticAnalyzer &context) {
    assert(value && type);
    assert(isExplicitlyConvertible(value->getType(), type));
    auto code = llvm::CastInst::getCastOpcode(value, true, type, true);
    if (llvm::isa<llvm::ConstantInt>(value)) {
        auto constant = llvm::cast<llvm::ConstantInt>(value);
        if (type->isIntegerTy())
            value = llvm::ConstantInt::get(llvm::cast<llvm::IntegerType>(type), constant->getLimitedValue(), true);
        else if (type->isPointerTy() && constant->isNullValue()) {
            value = llvm::ConstantInt::get(llvm::cast<llvm::IntegerType>(type), constant->getLimitedValue(), true);
        }
    }
    value = llvm::CastInst::Create(code, value, type, "", context.block());
}
