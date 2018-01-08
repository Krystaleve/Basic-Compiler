#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/Support/raw_ostream.h>
#include <iostream>
#include "expression.h"
#include "type.h"

YacConstantExpression::YacConstantExpression(llvm::Value *value)
    : value(value) {}

llvm::Value *YacConstantExpression::generateRvalue(YacCodeGenContext &context) {
    if (value->getType()->isArrayTy()) {
        // String literature
        auto variable = new llvm::GlobalVariable(context.module(), value->getType(), true, llvm::GlobalVariable::PrivateLinkage,
                                                 llvm::cast<llvm::Constant>(value), "");
        return llvm::GetElementPtrInst::CreateInBounds(variable, {
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(globalContext), 0),
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(globalContext), 0)
        }, "", context.block());
    }
    return value;
}


YacIdentifierExpression::YacIdentifierExpression(std::string *identifier)
    : identifier(identifier) {}

llvm::Value *YacIdentifierExpression::find(YacCodeGenContext &context) {
    assert(identifier);
    if (!context.is_top_level()) {
        auto iter = context.locals().find(*identifier);
        if (iter != context.locals().end())
            return iter->second;
    }
    auto iter = context.globals().find(*identifier);
    if (iter != context.globals().end())
        return iter->second;
    std::cerr << "use of undeclared identifier \'" << *identifier << "\'" << std::endl;
    return nullptr;
}

llvm::Value *YacIdentifierExpression::generateLvalue(YacCodeGenContext &context) {
    return find(context);
}

llvm::Value *YacIdentifierExpression::generateRvalue(YacCodeGenContext &context)
{
    auto variable = find(context);
    if (variable) {
        auto value_type = llvm::cast<llvm::PointerType>(variable->getType())->getElementType();
        if (value_type->isFunctionTy())
            return variable;
        if (value_type->isArrayTy())
            return llvm::GetElementPtrInst::CreateInBounds(variable, {
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(globalContext), 0),
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(globalContext), 0)
            }, "", context.block());
        return new llvm::LoadInst(variable, "", context.block());
    }
    return variable;
}


YacCallExpression::YacCallExpression(YacExpression *func, YacExpressionList *args)
    : func(func), args(args) {}

llvm::FunctionType *YacCallExpression::getFunc(llvm::Value *function, YacCodeGenContext &context) {
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

llvm::Value *YacCallExpression::doGenerate(llvm::Value *function, llvm::FunctionType *function_type, YacCodeGenContext &context) {
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
                arguments.push_back(castToType(value, *iter++, context));
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
                arguments.push_back(castToType(value, *iter++, context));
            }
    }
    return llvm::CallInst::Create(function, arguments, "", context.block());
}

llvm::Value *YacCallExpression::generate(YacCodeGenContext &context) {
    auto function = func->generateRvalue(context);
    if (!function)
        return nullptr;
    auto function_type = getFunc(function, context);
    if (!function_type)
        return nullptr;
    return doGenerate(function, function_type, context);
}

llvm::Value *YacCallExpression::generateRvalue(YacCodeGenContext &context) {
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

