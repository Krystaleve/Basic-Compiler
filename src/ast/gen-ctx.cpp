#include <llvm/IR/PassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include "gen-ctx.h"

llvm::LLVMContext globalContext; // NOLINT

YacCodeGenContext::YacCodeGenContext()
    : m_module("main", globalContext) {}


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

llvm::BasicBlock* YacCodeGenContext::current_block() {
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

void YacCodeGenContext::print_code() {
    llvm::PassManager<llvm::Module> pm;
    llvm::AnalysisManager<llvm::Module> am;
    pm.addPass(llvm::PrintModulePass(llvm::outs()));
    pm.run(m_module, am);
}

