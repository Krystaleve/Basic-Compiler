#include "expression.h"

YacPrimaryExpression::YacPrimaryExpression(llvm::Value *value)
    : value(value) {}

llvm::Value *YacPrimaryExpression::generateRvalue(YacCodeGenContext &context) {
    return value;
}

