#ifndef CAST_H_INCLUDE
#define CAST_H_INCLUDE

#include <llvm/IR/DerivedTypes.h>

bool isValidVariableType(llvm::Type *type);
bool isValidFunctionType(llvm::FunctionType *type);

llvm::Type *castToParameterType(llvm::Type *type);

#endif