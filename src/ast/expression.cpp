#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/Support/raw_ostream.h>
#include <iostream>
#include "ast.h"
#include "declaration.h"
#include "expression.h"
#include "type.h"

YacConstantExpression::YacConstantExpression(llvm::Value *value)
    : value(value) {}

llvm::Value *YacConstantExpression::generateRvalue(YacSemanticAnalyzer &context) {
    if (isString()) {
        // String literature
        auto variable = new llvm::GlobalVariable(context.module(), value->getType(), true, llvm::GlobalVariable::PrivateLinkage,
                                                 llvm::cast<llvm::Constant>(value), "");
        return llvm::GetElementPtrInst::CreateInBounds(variable, {
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(YacSemanticAnalyzer::context()), 0),
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(YacSemanticAnalyzer::context()), 0)
        }, "", context.block());
    }
    return value;
}


llvm::Value *YacLvalueExpression::generateRvalue(YacSemanticAnalyzer &context) {
    return castLvalueToRvalue(generateLvalue(context), context);
}


YacObjectExpression::YacObjectExpression(YacDeclaration *declaration)
    : declaration(declaration) { assert(declaration); }

llvm::Value *YacObjectExpression::generateLvalue(YacSemanticAnalyzer &context) {
    auto variable = context.find(declaration);
    if (!variable)  // should not happen
        std::cerr << "yac: " << YacSemanticError("object does not exist", this) << std::endl;
    return variable;
}

YacCallExpression::YacCallExpression(YacExpression *func, YacExpressionList *args)
    : func(func), args(args) {}

llvm::FunctionType *YacCallExpression::getFunc(llvm::Value *function, YacSemanticAnalyzer &context) {
    if (!function->getType()->isPointerTy()) {
        std::cerr << "called object type is not a function or function pointer" << std::endl;
        return nullptr;
    }
    auto value_type = llvm::cast<llvm::PointerType>(function->getType())->getElementType();
    if (!value_type->isFunctionTy()) {
        std::cerr << "called object type is not a function or function pointer" << std::endl;
        return nullptr;
    }
    return llvm::cast<llvm::FunctionType>(value_type);
}

llvm::Value *YacCallExpression::doGenerate(llvm::Value *function, llvm::FunctionType *function_type, YacSemanticAnalyzer &context) {
    std::vector<llvm::Value *> arguments;
    if (function_type->isVarArg()) {
        // at least one fix
        if (args == nullptr || args->size() < function_type->getNumParams()) {
            std::cerr << "wrong number of arguments" << std::endl;
            return nullptr;
        }
        auto params = function_type->params();
        auto iter = params.begin();
        for (auto arg: *args) {
            auto value = arg->generateRvalue(context);
            if (!value)
                return nullptr;
            if (iter != params.end())
                arguments.push_back(castValueToType(value, *iter++, context));
            else
                arguments.push_back(value);
        }
    } else {
        if ((args == nullptr && function_type->getNumParams() != 0) ||
                (args != nullptr && args->size() != function_type->getNumParams())) {
            std::cerr << "wrong number of arguments" << std::endl;
            return nullptr;
        }
        auto params = function_type->params();
        auto iter = params.begin();
        if (args)
            for (auto arg: *args) {
                auto value = arg->generateRvalue(context);
                if (!value)
                    return nullptr;
                arguments.push_back(castValueToType(value, *iter++, context));
            }
    }
    return llvm::CallInst::Create(function, arguments, "", context.block());
}

llvm::Value *YacCallExpression::generate(YacSemanticAnalyzer &context) {
    auto function = func->generateRvalue(context);
    if (!function)
        return nullptr;
    auto function_type = getFunc(function, context);
    if (!function_type)
        return nullptr;
    return doGenerate(function, function_type, context);
}

llvm::Value *YacCallExpression::generateRvalue(YacSemanticAnalyzer &context) {
    auto function = func->generateRvalue(context);
    if (!function)
        return nullptr;
    auto function_type = getFunc(function, context);
    if (!function_type)
        return nullptr;
    if (function_type->getReturnType()->isVoidTy()) {
        std::cerr << "access void value" << std::endl;
        return nullptr;
    }
    return doGenerate(function, function_type, context);
}

YacBinaryExpression::YacBinaryExpression(YacExpression *left, YacExpression *right, int token)
        : left(left), right(right), token(token) {}

llvm::Value *YacBinaryExpression::generateRvalue(YacSemanticAnalyzer &context)
{
    auto left_value = left->generateRvalue(context);
    auto right_value = right->generateRvalue(context);
    return binaryExpression(left_value, right_value, token, context);
}

YacAssignmentExpression::YacAssignmentExpression(YacExpression *left, YacExpression *right)
      : left(left), right(right) {}

llvm::Value *YacAssignmentExpression::generateLvalue(YacSemanticAnalyzer &context) {
    auto left_value = left->generateLvalue(context);
    if (!left_value)
        return nullptr;
    auto right_value = right->generateRvalue(context);
    if (!right_value)
        return nullptr;
    new llvm::StoreInst(castValueToType(right_value, llvm::cast<llvm::PointerType>(left_value->getType())->getElementType(), context), left_value, context.block());
    return left_value;
}


YacCompoundAssignmentExpression::YacCompoundAssignmentExpression(YacExpression *left, YacExpression *right, int token)
        : left(left), right(right), token(token) {}

llvm::Value *YacCompoundAssignmentExpression::generateLvalue(YacSemanticAnalyzer &context) {
    auto left_value = left->generateLvalue(context);
    if (!left_value)
        return nullptr;
    auto left_value_rvalue = castLvalueToRvalue(left_value, context);
    if (!left_value_rvalue)
        return nullptr;
    auto result = binaryExpression(left_value_rvalue, right->generateRvalue(context), token, context);
    if (!result)
        return nullptr;
    new llvm::StoreInst(castValueToType(result, llvm::cast<llvm::PointerType>(left_value->getType())->getElementType(), context), left_value, context.block());
    return left_value;
}


bool binaryExpressionCheck(llvm::Value *left, llvm::Value *right, int token) {
    if (!left || !right)
        return false;
    // TODO
    // auto left_type = left->getType(), right_type = right->getType();
    return false;
}

llvm::Value *binaryExpression(llvm::Value *left, llvm::Value *right, int token, YacSemanticAnalyzer &contex) {
    return nullptr;
}
