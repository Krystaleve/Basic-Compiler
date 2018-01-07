#include "expression.h"

YacPrimaryExpression::YacPrimaryExpression(llvm::Value *value)
    : value(value) {}

llvm::Value *YacPrimaryExpression::generate(YacCodeGenContext &context) {
    return value;
}

