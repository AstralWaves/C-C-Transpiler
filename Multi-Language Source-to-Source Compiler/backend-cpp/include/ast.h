#ifndef AST_H
#define AST_H

#include <vector>
#include <string>
#include <memory>

enum NodeType {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_VARIABLE_DECL,
    NODE_ASSIGNMENT,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_IF_STATEMENT,
    NODE_WHILE_LOOP,
    NODE_FOR_LOOP,
    NODE_SWITCH_STATEMENT,
    NODE_CASE_STATEMENT,
    NODE_BREAK_STATEMENT,
    NODE_CONTINUE_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_CALL_EXPRESSION,
    NODE_NUMBER_LITERAL,
    NODE_STRING_LITERAL,
    NODE_IDENTIFIER,
    NODE_PARAMETER,
    NODE_BLOCK,
    NODE_EXPRESSION
};

struct ASTNode {
    NodeType type;
    std::string value;
    std::vector<std::unique_ptr<ASTNode>> children;
    ASTNode* parent;
    int line;
    int column;
    
    ASTNode(NodeType t, const std::string& v = "", int l = 0, int c = 0)
        : type(t), value(v), parent(nullptr), line(l), column(c) {}
    
    void addChild(std::unique_ptr<ASTNode> child) {
        child->parent = this;
        children.push_back(std::move(child));
    }
};

// JSON serialization for frontend
class ASTSerializer {
public:
    static std::string toJson(ASTNode* node);
private:
    static void serialize(ASTNode* node, std::stringstream& ss, int indent = 0);
};

#endif