#include <llvm/IR/Type.h>
#include <llvm/IR/Instructions.h>
#include <iostream>
#include "statement.h"
#include "context.h"

YacReturnStatement::YacReturnStatement(YacSyntaxTreeNode *expression)
    :expression(expression) {}

llvm::Value *YacReturnStatement::generate(YacCodeGenContext &context)
{
    if (expression == nullptr) {
        if (context.function()->getReturnType()->isVoidTy())
            return llvm::ReturnInst::Create(globalContext, context.block());
        std::cerr << "Non-void function should return a value" << std::endl;
    } else {
        if (!context.function()->getReturnType()->isVoidTy()) {
            auto value = expression->generate(context);
            if (!value)
                return nullptr;
            // TODO: type check
            return llvm::ReturnInst::Create(globalContext, value, context.block());
        }
        std::cerr << "Void function should not return a value" << std::endl;
    }
    return nullptr;
}

