#include <llvm/IR/Value.h>
#include <vector>
#include "declaration.h"
#include "context.h"

unsigned int line_number = 1;
unsigned int column_number = 1;
std::string input("<stdin>"); // NOLINT

YacScope *root = new YacScope; // NOLINT
static std::vector<YacScope *> scopes{root}; // NOLINT

llvm::Value *YacSyntaxTreeNodeList::generate(YacSemanticAnalyzer &context)
{
    for (auto child: children)
        child->generate(context);
    return nullptr;
}

std::ostream &operator << (std::ostream &out, const YacSyntaxError &err) {
    out << err.pos << ": " << err.name() << " error: " << err.what();
    return out;
}

std::ostream &operator << (std::ostream &out, const YacSemanticError &err) {
    auto source = err.source();
    out << source->pos << ": " << err.name() << " error: " << err.what();
    return out;
}

void YacScope::addToScope(YacDeclaration *declaration)
{
    assert(declaration && declaration->identifier);
    // TODO redeclaration
    if (declarations.find(*declaration->identifier) != declarations.end())
        std::cerr << "yac: " << YacSyntaxError("identifier redeclaration") << std::endl;
    declarations.insert(std::make_pair(*declaration->identifier, declaration));
}

void pushScope(YacScope *scope) {
    scopes.push_back(scope);
#ifdef DEBUG_SCOPE
    std::cerr << "yac: debug: " << YacPos() << (" { #" + std::to_string(scopes.size() - 1)) << std::endl;
#endif
}
YacScope *topScope() {
    return scopes.empty() ? nullptr : scopes.back();
}
void popScope() {
    assert(!scopes.empty());
    scopes.pop_back();
#ifdef DEBUG_SCOPE
    std::cerr << "yac: debug: " << YacPos() << (" } #" + std::to_string(scopes.size())) << std::endl;
#endif
}

bool addToTopScope(YacDeclaration *declaration)
{
    auto scope = topScope();
    if (declaration && declaration->identifier && scope) {
        scope->addToScope(declaration);
#ifdef DEBUG_SCOPE
        std::cerr << "yac: debug: " << YacPos() << (" add " + *declaration->identifier + " #" + std::to_string(scopes.size())) << std::endl;
#endif
    }
    return false;
}


YacDeclaration *findInScopes(const std::string &identifier) {
    for (auto iter = scopes.rbegin(); iter < scopes.rend(); ++iter) {
        if (!*iter)
            continue;
        auto &declarations = (*iter)->declarations;
        auto search = declarations.find(identifier);
        if (search != declarations.end()) {
#ifdef DEBUG_SCOPE
            std::cerr << "yac: debug: " << YacPos() << (" hit " + identifier + " #" + std::to_string((iter - scopes.rbegin()))) << std::endl;
#endif
            return search->second;
        }
    }
    return nullptr;
}

