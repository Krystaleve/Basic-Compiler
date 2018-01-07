#include <llvm/IR/PassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <iostream>
#include "context.h"

llvm::LLVMContext globalContext; // NOLINT

YacCodeGenContext::YacCodeGenContext()
    : m_module(new llvm::Module("main", globalContext)), m_function(nullptr) {}


CodeBlock::CodeBlock(llvm::BasicBlock *block)
    : m_block(block) {}

CodeBlock::CodeBlock(llvm::BasicBlock *block, const std::map<std::string, llvm::Value *> &locals) // NOLINT
    : m_block(block), m_locals(locals) {}

std::map<std::string, llvm::Value*>& YacCodeGenContext::locals() {
    return m_blocks.top().m_locals;
}

std::map<std::string, llvm::Value*>& YacCodeGenContext::globals() {
    return m_globals;
}

llvm::BasicBlock* YacCodeGenContext::block() {
    return m_blocks.top().m_block;
}

void YacCodeGenContext::push_block(llvm::BasicBlock *block, bool copy_locals) {
    if (copy_locals)
        m_blocks.emplace(block, m_blocks.top().m_locals);
    else
        m_blocks.emplace(block);
}

void YacCodeGenContext::pop_block() {
    m_blocks.pop();
}

void YacCodeGenContext::print() {
    llvm::PassManager<llvm::Module> pm;
    llvm::AnalysisManager<llvm::Module> am;
    pm.addPass(llvm::PrintModulePass(llvm::outs()));
    pm.run(*m_module, am);
}

int YacCodeGenContext::execute() {
    auto main = m_module->getFunction("main");
    if (!main) {
        std::cerr << "Cannot find main function" << std::endl;
        return 1;
    }
    std::vector<llvm::GenericValue> args;
    if (main->getFunctionType() != llvm::FunctionType::get(llvm::IntegerType::get(globalContext, 32), false)) {
        std::cerr << "Wrong main function signature" << std::endl;
        return 1;
    }
    llvm::ExecutionEngine *engine = llvm::EngineBuilder(std::move(m_module)).create();
    if (!engine) {
        std::cerr << "Failed to create execution engine" << std::endl;
        return 1;
    }
    engine->finalizeObject();
    llvm::GenericValue v = engine->runFunction(main, args);
    return static_cast<int>(v.IntVal.getLimitedValue());
}