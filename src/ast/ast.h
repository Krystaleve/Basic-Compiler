#ifndef AST_H_INCLUDE
#define AST_H_INCLUDE

#include <llvm/IR/Value.h>
#include <vector>
#include <exception>
#include <string>
#include <stack>
#include <map>

// #define DEBUG_SCOPE

extern std::string input;
extern unsigned int line_number;
extern unsigned int column_number;

class YacPos {
    friend std::ostream &operator << (std::ostream &out, const YacPos &pos);
public:
    YacPos() {
        m_i = input;
        m_l = line_number;
        m_c = column_number;
    }
private:
    std::string m_i;
    unsigned int m_l, m_c;
};

inline std::ostream &operator << (std::ostream &out, const YacPos &pos) {
    out << pos.m_i << ':' << pos.m_l << ':' << pos.m_c;
    return out;
}

class YacDeclaration;
class YacSemanticAnalyzer;
class YacScope;

class YacSyntaxTreeNode {
public:
    YacPos pos;
    virtual ~YacSyntaxTreeNode() = default;
    virtual llvm::Value* generate(YacSemanticAnalyzer &context) {
        return nullptr;
    }
};

class YacSyntaxTreeNodeList: public YacSyntaxTreeNode {
public:
    std::vector<YacSyntaxTreeNode *> children;
    void addNode(YacSyntaxTreeNode *node) {
        if (node)
            children.push_back(node);
    }
    llvm::Value* generate(YacSemanticAnalyzer &context) override;
};

class YacSyntaxError: public std::runtime_error {
public:
    YacPos pos;

    explicit YacSyntaxError(const std::string& arg): std::runtime_error(arg) {}
    virtual std::string name() const {
        return "syntax";
    }
};

class YacSemanticError: public std::runtime_error {
public:
    explicit YacSemanticError(const std::string& arg, YacSyntaxTreeNode *source): std::runtime_error(arg), m_source(source) {}
    virtual std::string name() const {
        return "semantic";
    }
    YacSyntaxTreeNode *source() const { return m_source; }
    std::vector<YacSyntaxTreeNode *> &path() { return m_path; }
    const std::vector<YacSyntaxTreeNode *> &path() const { return m_path; }
private:
    YacSyntaxTreeNode *m_source;
    std::vector<YacSyntaxTreeNode *> m_path;
};

std::ostream &operator << (std::ostream &out, const YacSyntaxError &err);
std::ostream &operator << (std::ostream &out, const YacSemanticError &err);


class YacScope: public YacSyntaxTreeNodeList {
public:
    void addToScope(YacDeclaration *declaration);
    std::map<std::string, YacDeclaration *> declarations;
};


extern YacScope *root;

void pushScope(YacScope *scope = nullptr);
YacScope *topScope();
void popScope();
bool addToTopScope(YacDeclaration *declaration);
YacDeclaration *findInScopes(const std::string &identifier);

#endif