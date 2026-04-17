#ifndef INTERMEDIATE_CODE_H
#define INTERMEDIATE_CODE_H

#include <string>
#include <vector>
#include "ast.h"

struct Quadruple {
    enum Op {
        OP_ASSIGN,
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_MOD,
        OP_LT,
        OP_GT,
        OP_LE,
        OP_GE,
        OP_EQ,
        OP_NE,
        OP_AND,
        OP_OR,
        OP_NOT,
        OP_JMP,
        OP_JMP_TRUE,
        OP_JMP_FALSE,
        OP_CALL,
        OP_RET,
        OP_PARAM,
        OP_LABEL,
        OP_IF,
        OP_ELSE,
        OP_WHILE,
        OP_FOR,
        OP_PRINTF,
        OP_SCANF,
        OP_RETURN,
        OP_FUNC_START,
        OP_FUNC_END
    };
    
    Op op;
    std::string arg1;
    std::string arg2;
    std::string result;
    
    Quadruple(Op o, const std::string& a1 = "", const std::string& a2 = "", const std::string& res = "")
        : op(o), arg1(a1), arg2(a2), result(res) {}
    
    std::string opToString() const;
};

class IntermediateCodeGenerator {
public:
    std::vector<Quadruple> generate(ASTNode* ast);
};

#endif