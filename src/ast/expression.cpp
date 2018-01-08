#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/Support/raw_ostream.h>
#include <iostream>
#include "ast.h"
#include "declaration.h"
#include "expression.h"
#include "type.h"
// For token
#include "../syntax/syntax.h"

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


llvm::Value *YacLvalueExpression::generateRvalue(YacCodeGenContext &context) {
    return castLvalueToRvalue(generateLvalue(context), context);
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

llvm::Value *YacIdentifierExpression::generate(YacCodeGenContext &context) {
    find(context);
    return nullptr;
}


llvm::Value *YacIdentifierExpression::generateLvalue(YacCodeGenContext &context) {
    auto variable = find(context);
    if (variable) {
        auto value_type = llvm::cast<llvm::PointerType>(variable->getType())->getElementType();
        if (value_type->isFunctionTy()) {
            std::cerr << "cannot use function as lvalue" << std::endl;
            return nullptr;
        }
        if (value_type->isArrayTy()) {
            std::cerr << "cannot use array as lvalue" << std::endl;
            return nullptr;
        }
    }
    return variable;
}

llvm::Value *YacIdentifierExpression::generateRvalue(YacCodeGenContext &context) {
    return castLvalueToRvalue(find(context), context);
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

YacBinaryExpression::YacBinaryExpression(YacExpression *left, YacExpression *right, int token)
        : left(left), right(right), token(token)
{}

llvm::Value *YacBinaryExpression::generate(YacCodeGenContext &context) {
    auto left_value = left->generateRvalue(context);
    auto right_value = right->generateRvalue(context);
    binaryExpressionCheck(left_value, right_value, token);
    return nullptr;
}

llvm::Value *YacBinaryExpression::generateRvalue(YacCodeGenContext &context)
{
    auto left_value = left->generateRvalue(context);
    auto right_value = right->generateRvalue(context);
    return binaryExpression(left_value, right_value, token, context);
}


YacBinaryLogicExpression::YacBinaryLogicExpression(YacExpression *left, YacExpression *right, int token)
        : left(left), right(right), token(token) {}

llvm::Value *YacBinaryLogicExpression::generate(YacCodeGenContext &context) {
    // TODO
    return YacExpression::generate(context);
}

llvm::Value *YacBinaryLogicExpression::generateRvalue(YacCodeGenContext &context)
{
    // TODO
    return YacExpression::generateRvalue(context);
}


YacAssignmentExpression::YacAssignmentExpression(YacExpression *left, YacExpression *right)
    : left(left), right(right) {}

llvm::Value *YacAssignmentExpression::generate(YacCodeGenContext &context) {
    if(left->generateLvalue(context))
        right->generateRvalue(context);
    return nullptr;
}

llvm::Value *YacAssignmentExpression::generateLvalue(YacCodeGenContext &context) {
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

llvm::Value *YacCompoundAssignmentExpression::generate(YacCodeGenContext &context) {
    auto left_value = left->generateLvalue(context);
    if (!left_value)
        return nullptr;
    auto left_value_rvalue = castLvalueToRvalue(left_value, context);
    if (!left_value_rvalue)
        return nullptr;
    auto right_value = right->generateRvalue(context);
    binaryExpressionCheck(left_value_rvalue, right_value, token);
    return nullptr;
}

llvm::Value *YacCompoundAssignmentExpression::generateLvalue(YacCodeGenContext &context) {
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
    auto left_type = left->getType(), right_type = right->getType();
    switch (token) {
        case '+':
            if (isArithmeticType(left_type) && isArithmeticType(right_type)) {

            }
        default:
            return false;
    }
}

llvm::Value *binaryExpression(llvm::Value *left, llvm::Value *right, int token, YacCodeGenContext &contex) {
    return nullptr;
}
