#ifndef CAST_H_INCLUDE
#define CAST_H_INCLUDE

#include <llvm/IR/DerivedTypes.h>

#include "context.h"

bool isValidVariableType(llvm::Type *type);
bool isValidFunctionType(llvm::FunctionType *type);

llvm::Type *castToParameterType(llvm::Type *type);

llvm::Value *castToType(llvm::Value *value, llvm::Type *type, YacCodeGenContext &context);

#endif