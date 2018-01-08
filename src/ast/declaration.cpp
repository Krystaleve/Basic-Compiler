#include <iostream>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/Support/raw_ostream.h>
#include "declaration.h"
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


YacDeclaratorFunction::YacDeclaratorFunction(YacDeclaratorBuilder *parent, YacSyntaxTreeNode *node, bool var_arg)
        : YacDeclaratorHasParent(parent), m_node(node), m_var_arg(var_arg) {}

llvm::Type *YacDeclaratorFunction::type(llvm::Type *specifier)
{
    std::vector<llvm::Type *> params;
    if (m_node)
        for (auto node: dynamic_cast<YacSyntaxTreeNodeList *>(m_node)->children)
            params.push_back(dynamic_cast<YacDeclaration *>(node)->type);
    return YacDeclaratorHasParent::type(llvm::FunctionType::get(specifier, params, m_var_arg));
}


YacDeclaration::YacDeclaration(llvm::Type *type, std::string *identifier)
    : type(type), identifier(identifier) {}

llvm::Value *YacDeclaration::generate(YacCodeGenContext &context)
{
    if (identifier == nullptr) {
        std::cerr << "name omitted" << std::endl;
        return nullptr;
    }
    auto &scope = context.is_top_level() ? context.globals() : context.locals();
    if (scope.find(*identifier) != scope.end()) {
        std::cerr << "redefinition of \'" << *identifier << "\'" << std::endl;
        return nullptr;
    }
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
        if (context.is_top_level()) {
            llvm::Constant *init = llvm::Constant::getNullValue(type);
            var = new llvm::GlobalVariable(context.module(), type, false, llvm::GlobalVariable::CommonLinkage,
                                           init, *identifier);
        } else
            var = new llvm::AllocaInst(type, 0, "", context.block());
    }
    scope.insert(std::make_pair(*identifier, var));
    return var;
}


YacFunctionDefinition::YacFunctionDefinition(llvm::FunctionType *type, std::string *identifier, std::vector<YacDeclaration *> &&params, YacSyntaxTreeNode *body)
    : type(type), identifier(identifier), params(params), body(body) {}

llvm::Value *YacFunctionDefinition::generate(YacCodeGenContext &context)
{
    assert(identifier);
    auto &scope = context.is_top_level() ? context.globals() : context.locals();
    if (scope.find(*identifier) != scope.end()) {
        std::cerr << "redefinition of \'" << *identifier << "\'" << std::endl;
        return nullptr;
    }
    if (!isValidFunctionType(type))
        return nullptr;
    auto function = llvm::Function::Create(type, llvm::GlobalValue::ExternalLinkage, *identifier, &context.module());
    scope.insert(std::make_pair(*identifier, function));
    auto block = llvm::BasicBlock::Create(globalContext, "", function);
    context.setFunction(function);
    context.push_block(block);
    auto arg_values = function->arg_begin();
    for (auto param: params) {
        auto variable = param->generate(context);
        if (variable)
            new llvm::StoreInst(arg_values++, variable, block);
    }
    if (body)
        body->generate(context);
    if (!block->getTerminator()) {
        if (function->getReturnType()->isVoidTy()) {
            llvm::ReturnInst::Create(globalContext, block);
        } else {
            llvm::Value *variable = new llvm::AllocaInst(function->getReturnType(), 0, "", block);
            llvm::Value *value = new llvm::LoadInst(variable, "", block);
            llvm::ReturnInst::Create(globalContext, value, block);
        }
    }
    context.pop_block();
    context.setFunction(nullptr);
    return function;
}
