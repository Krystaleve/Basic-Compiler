#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>
#include <iostream>
#include "statement.h"
#include "context.h"
#include "type.h"

YacReturnStatement::YacReturnStatement(YacExpression *expression)
    :expression(expression) {}

llvm::Value *YacReturnStatement::generate(YacSemanticAnalyzer &context)
{
    if (expression == nullptr) {
        if (context.function()->getReturnType()->isVoidTy())
            return llvm::ReturnInst::Create(YacSemanticAnalyzer::context(), context.block());
        std::cerr << "Non-void function should return a value" << std::endl;
    } else {
        if (!context.function()->getReturnType()->isVoidTy()) {
            auto value = expression->generateRvalue(context);
            if (!value)
                return nullptr;
            return llvm::ReturnInst::Create(YacSemanticAnalyzer::context(),
                                            castValueToType(value, context.function()->getReturnType(), context), context.block());
        }
        std::cerr << "Void function should not return a value" << std::endl;
    }
    return nullptr;
}

