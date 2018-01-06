#ifndef GENCTX_H_INCLUDE
#define GENCTX_H_INCLUDE

#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <stack>
#include <map>
#include "ast.h"

extern llvm::LLVMContext globalContext;

class CodeBlock {
    friend class YacCodeGenContext;
public:
    explicit CodeBlock(llvm::BasicBlock *block);
    CodeBlock(llvm::BasicBlock *block, const std::map<std::string, llvm::Value *> &locals);
private:
    llvm::BasicBlock *m_block;
    std::map<std::string, llvm::Value *> m_locals;
};


class YacCodeGenContext {
public:
    YacCodeGenContext();

    std::map<std::string, llvm::Value*>& locals();
    std::map<std::string, llvm::Value*>& globals();

    llvm::BasicBlock* current_block();
    void push_block(llvm::BasicBlock* block, bool copy_locals = true);
    void pop_block();
    bool is_top_level() {
        return m_blocks.empty();
    }

    void generate_code(YacSyntaxTreeNodeList *root);
private:
    std::stack<CodeBlock> m_blocks;
    std::map<std::string, llvm::Value *> m_globals;

    llvm::Module m_module;
};

#endif