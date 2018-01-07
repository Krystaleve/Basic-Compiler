
#include <llvm/IR/Value.h>
#include "gen-ctx.h"

llvm::Value *YacSyntaxTreeNodeList::generate(YacCodeGenContext &context)
{
    for (auto child: children)
        child->generate(context);
    return nullptr;
}
