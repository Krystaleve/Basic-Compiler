#include <llvm/IR/Value.h>
#include "context.h"

llvm::Value *YacSyntaxTreeNodeList::generate(YacCodeGenContext &context)
{
    for (auto child: children)
        if (child)
            child->generate(context);
    return nullptr;
}
