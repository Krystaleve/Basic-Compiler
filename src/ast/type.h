#ifndef CAST_H_INCLUDE
#define CAST_H_INCLUDE

#include <llvm/IR/DerivedTypes.h>

#include "context.h"

bool isValidVariableType(llvm::Type *type);
bool isValidFunctionType(llvm::FunctionType *type);

llvm::Type *castToParameterType(llvm::Type *type);

llvm::Value *castValueToType(llvm::Value *value, llvm::Type *type, YacSemanticAnalyzer &context);

llvm::Value *castLvalueToRvalue(llvm::Value *value, YacSemanticAnalyzer &context);

std::string getTypeName(llvm::Type *type);

/******** Following functions ensure to be ANSI C subset in our system ********/
// It is always asserted that all inputs are valid (not null, valid value category etc.).
// And it can be asserted that all output are also valid.

/**** Type Category ****/
bool isObjectType(llvm::Type *type);
bool isArithmeticType(llvm::Type *type);
bool isNull(llvm::Value *value);

/**** Conversion ****/
bool isCompatible(llvm::Type *left, llvm::Type *right);
bool isImplicitlyConvertible(llvm::Value *value, llvm::Type *dst_type);
bool isExplicitlyConvertible(llvm::Type *src_type, llvm::Type *dst_type);
void usualArithmeticConversions(llvm::Value *&left, llvm::Value *&right, YacSemanticAnalyzer &context);

/**** Expression ****/
void cast(llvm::Value *&value, llvm::Type *type, YacSemanticAnalyzer &context);

#endif