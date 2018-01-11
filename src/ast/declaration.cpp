#include <iostream>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/Support/raw_ostream.h>
#include "declaration.h"
#include "expression.h"
#include "context.h"
#include "type.h"

YacDeclaratorIdentifier::YacDeclaratorIdentifier(std::string *identifier)
    : m_identifier(identifier) {}

std::string *YacDeclaratorIdentifier::identifier() {
    return m_identifier;
}


YacDeclaratorHasParent::YacDeclaratorHasParent(YacDeclaratorBuilder *parent)
        : parent(parent) {}

llvm::Type *YacDeclaratorHasParent::type(llvm::Type *specifier) {
    return parent->type(specifier);
}

std::string *YacDeclaratorHasParent::identifier() {
    return parent->identifier();
}


YacDeclaratorPointer::YacDeclaratorPointer(YacDeclaratorBuilder *parent)
    : YacDeclaratorHasParent(parent)  {}

llvm::Type *YacDeclaratorPointer::type(llvm::Type *specifier) {
    return YacDeclaratorHasParent::type(llvm::PointerType::getUnqual(specifier));
}


YacDeclaratorArray::YacDeclaratorArray(YacDeclaratorBuilder *parent, uint64_t num)
    : YacDeclaratorHasParent(parent), m_num(num) {}

llvm::Type *YacDeclaratorArray::type(llvm::Type *specifier) {
    return YacDeclaratorHasParent::type(llvm::ArrayType::get(specifier, m_num));
}


YacDeclaratorFunction::YacDeclaratorFunction(YacDeclaratorBuilder *parent, YacDeclarationList *node, bool var_arg)
        : YacDeclaratorHasParent(parent), m_node(node), m_var_arg(var_arg) {}

llvm::Type *YacDeclaratorFunction::type(llvm::Type *specifier)
{
    std::vector<llvm::Type *> params;
    if (m_node)
        for (auto node: m_node->children)
            params.push_back(node->type);
    return YacDeclaratorHasParent::type(llvm::FunctionType::get(specifier, params, m_var_arg));
}


YacDeclaration::YacDeclaration(llvm::Type *type, std::string *identifier, int specifier)
    : YacSyntaxTreeNode(), type(type), identifier(identifier), specifier(specifier) {
    assert(type);
}

llvm::Value *YacDeclaration::generate(YacSemanticAnalyzer &context)
{
    llvm::Value *var;
    if (type->isFunctionTy()) {
        auto function_type = llvm::cast<llvm::FunctionType>(type);
        if (!isValidFunctionType(function_type))
            return nullptr;
        var = llvm::Function::Create(function_type, llvm::GlobalValue::ExternalLinkage,
                                     *identifier, &context.module());
    } else {
        if (!isValidVariableType(type))
            return nullptr;
        auto block = context.block();
        if (!block) {
            llvm::Constant *init = llvm::Constant::getNullValue(type);
            var = new llvm::GlobalVariable(context.module(), type, false, llvm::GlobalVariable::CommonLinkage,
                                           init, *identifier);
        } else
            var = new llvm::AllocaInst(type, 0, "", block);
    }
    context.add(this, var);
    return var;
}


YacFunctionDefinition::YacFunctionDefinition(llvm::FunctionType *type, YacScope *params, YacSyntaxTreeNode *body, std::string *identifier, int specifier)
        : YacDeclaration(type, identifier, specifier), params(params), body(body) {}

llvm::Value *YacFunctionDefinition::generate(YacSemanticAnalyzer &context)
{
    auto type = llvm::cast<llvm::FunctionType>(this->type);
    if (!isValidFunctionType(type))
        return nullptr;
    assert(!context.function() && !context.block());
    auto function = llvm::Function::Create(type, llvm::GlobalValue::ExternalLinkage, identifier ? *identifier : "",
                                      &context.module());
    context.add(this, function);
    auto block = llvm::BasicBlock::Create(YacSemanticAnalyzer::context(), "", function);
    context.setFunction(function);
    context.setBlock(block);
    auto arg_values = function->arg_begin();
    if (params) {
        for (auto param: params->children) {
            auto variable = param->generate(context);
            if (variable)
                new llvm::StoreInst(arg_values, variable, block);
            ++arg_values;
        }
    }
    if (body)
        body->generate(context);
    context.ensureBlockTerminated();
    context.setBlock(nullptr);
    context.setFunction(nullptr);
    return function;
}
