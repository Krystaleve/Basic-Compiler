#ifndef CAST_H_INCLUDE
#define CAST_H_INCLUDE

#include <llvm/IR/DerivedTypes.h>

#include "context.h"

bool isValidVariableType(llvm::Type *type);
bool isValidFunctionType(llvm::FunctionType *type);

llvm::Type *castToParameterType(llvm::Type *type);

llvm::Value *castValueToType(llvm::Value *value, llvm::Type *type, YacCodeGenContext &context);

llvm::Value *castLvalueToRvalue(llvm::Value *value, YacCodeGenContext &context);

bool isArithmeticType(llvm::Type *type);

#endif