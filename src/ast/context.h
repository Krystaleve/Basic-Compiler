#ifndef GENCTX_H_INCLUDE
#define GENCTX_H_INCLUDE

#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <exception>
#include <stack>
#include <map>
#include <memory>
#include "ast.h"

class YacSemanticAnalyzer {
public:
    YacSemanticAnalyzer();

    static llvm::LLVMContext &context();

    llvm::Module &module() {
        return *m_module;
    }

    llvm::Value *find(YacDeclaration *declaration);
    void add(YacDeclaration *declaration, llvm::Value *value) {
        assert(value);
        m_values.insert(std::make_pair(declaration, value));
    }

    void print(llvm::raw_ostream &out);
    int execute(llvm::Function *main, int argc, const char **argv);

    llvm::BasicBlock *block() {
        return m_block;
    }
    void setBlock(llvm::BasicBlock *block) {
        m_block = block;
    }
    llvm::Function *function() {
        return m_function;
    }
    void setFunction(llvm::Function *function) {
        m_function = function;
    }
    void ensureBlockTerminated();

private:
    std::map<YacDeclaration *, llvm::Value *> m_values;
    std::unique_ptr<llvm::Module> m_module;
    llvm::BasicBlock *m_block;
    llvm::Function *m_function;
    static llvm::LLVMContext *g_context;
};


class YacFunctionDefinition;

YacFunctionDefinition *addEntry(YacDeclaration *main);

#endif