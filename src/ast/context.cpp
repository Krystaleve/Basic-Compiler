#include <llvm/IR/PassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/Instructions.h>
#include <iostream>
#include "context.h"
#include "declaration.h"
#include "expression.h"

YacSemanticAnalyzer::YacSemanticAnalyzer()
    : m_module(new llvm::Module("main", YacSemanticAnalyzer::context())), m_block(nullptr), m_function(nullptr) {}


void YacSemanticAnalyzer::print(llvm::raw_ostream &out) {
    llvm::PassManager<llvm::Module> pm;
    llvm::AnalysisManager<llvm::Module> am;
    pm.addPass(llvm::PrintModulePass(out));
    pm.run(*m_module, am);
}

int YacSemanticAnalyzer::execute(llvm::Function *main, int argc, const char **argv) {
    llvm::ExecutionEngine *engine = llvm::EngineBuilder(std::move(m_module)).create();
    if (!engine) {
        std::cerr << "yac: failed to create execution engine" << std::endl;
        return 1;
    }
    engine->finalizeObject();
    int (*func)(int argc, const char **argv) = reinterpret_cast<int (*)(int, const char **)>(engine->getPointerToFunction(main));
    return func(argc, argv);
}


llvm::LLVMContext *YacSemanticAnalyzer::g_context;

llvm::LLVMContext &YacSemanticAnalyzer::context() {
    if (g_context)
        return *g_context;
    g_context = new llvm::LLVMContext;
    return *g_context;
}

llvm::Value *YacSemanticAnalyzer::find(YacDeclaration *declaration)
{
    auto iter = m_values.find(declaration);
    return iter == m_values.end() ? nullptr : iter->second;
}

void YacSemanticAnalyzer::ensureBlockTerminated()
{
    assert(!m_block || m_function);
    if (m_block && !m_block->getTerminator()) {
        if (m_function->getReturnType()->isVoidTy()) {
            llvm::ReturnInst::Create(YacSemanticAnalyzer::context(), m_block);
        } else {
            llvm::Value *variable = new llvm::AllocaInst(m_function->getReturnType(), 0, "", m_block);
            llvm::Value *value = new llvm::LoadInst(variable, "", m_block);
            llvm::ReturnInst::Create(YacSemanticAnalyzer::context(), value, m_block);
        }
    }
}

YacFunctionDefinition *addEntry(YacDeclaration *main)
{
    assert(main);
    llvm::LLVMContext &context = YacSemanticAnalyzer::context();
    llvm::Type *type1 = llvm::Type::getInt32Ty(context),
            *type2 = llvm::PointerType::getUnqual(llvm::PointerType::getUnqual(llvm::Type::getInt32Ty(context)));
    std::vector<llvm::Type *> types {type1, type2};
    auto arg1 = new YacDeclaration(type1), arg2 = new YacDeclaration(type2);
    auto params = new YacScope;
    params->addNode(arg1);
    params->addNode(arg2);
    auto func = new YacFunctionDefinition(
            llvm::FunctionType::get(type1, types, false),
            params, new YacCallExpression(new YacObjectExpression(main), new YacExpressionList{
                    new YacObjectExpression(arg1),
                    new YacObjectExpression(arg2)
            }));
    root->addNode(func);
    return func;
}
