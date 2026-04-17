#include "../include/intermediate_code.h"

std::string Quadruple::opToString() const {
    switch (op) {
        case OP_ASSIGN: return "ASSIGN";
        case OP_ADD: return "ADD";
        case OP_SUB: return "SUB";
        case OP_MUL: return "MUL";
        case OP_DIV: return "DIV";
        case OP_MOD: return "MOD";
        case OP_LT: return "LT";
        case OP_GT: return "GT";
        case OP_LE: return "LE";
        case OP_GE: return "GE";
        case OP_EQ: return "EQ";
        case OP_NE: return "NE";
        case OP_AND: return "AND";
        case OP_OR: return "OR";
        case OP_NOT: return "NOT";
        case OP_JMP: return "JMP";
        case OP_JMP_TRUE: return "JMP_TRUE";
        case OP_JMP_FALSE: return "JMP_FALSE";
        case OP_CALL: return "CALL";
        case OP_RET: return "RET";
        case OP_PARAM: return "PARAM";
        case OP_LABEL: return "LABEL";
        case OP_IF: return "IF";
        case OP_ELSE: return "ELSE";
        case OP_WHILE: return "WHILE";
        case OP_FOR: return "FOR";
        case OP_PRINTF: return "PRINTF";
        case OP_SCANF: return "SCANF";
        case OP_RETURN: return "RETURN";
        case OP_FUNC_START: return "FUNC_START";
        case OP_FUNC_END: return "FUNC_END";
        default: return "UNKNOWN";
    }
}

static std::string generateExpression(ASTNode* node) {
    if (!node) return "";
    switch (node->type) {
        case NODE_NUMBER_LITERAL:
        case NODE_STRING_LITERAL:
        case NODE_IDENTIFIER:
            return node->value;
        case NODE_UNARY_OP:
            return node->value + generateExpression(node->children[0].get());
        case NODE_BINARY_OP: {
            std::string left = generateExpression(node->children[0].get());
            std::string right = generateExpression(node->children[1].get());
            return left + " " + node->value + " " + right;
        }
        case NODE_CALL_EXPRESSION: {
            std::string out = node->value + "(";
            for (size_t i = 0; i < node->children.size(); i++) {
                if (i > 0) out += ", ";
                out += generateExpression(node->children[i].get());
            }
            out += ")";
            return out;
        }
        default:
            return "";
    }
}

static void generateNode(ASTNode* node, std::vector<Quadruple>& code) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM:
            for (auto& child : node->children) {
                generateNode(child.get(), code);
            }
            break;

        case NODE_FUNCTION: {
            std::string params = "";
            for (auto& child : node->children) {
                if (child->type == NODE_PARAMETER) {
                    if (!params.empty()) params += ", ";
                    params += child->value;
                }
            }
            code.emplace_back(Quadruple::OP_FUNC_START, params, "", node->value);
            for (auto& child : node->children) {
                if (child->type != NODE_PARAMETER) {
                    generateNode(child.get(), code);
                }
            }
            code.emplace_back(Quadruple::OP_FUNC_END, "", "", node->value);
            break;
        }

        case NODE_VARIABLE_DECL: {
            std::string initValue = "0";
            if (!node->children.empty()) {
                initValue = generateExpression(node->children[0].get());
            }
            code.emplace_back(Quadruple::OP_ASSIGN, initValue, "", node->value);
            break;
        }

        case NODE_ASSIGNMENT: {
            std::string expr = generateExpression(node->children[0].get());
            code.emplace_back(Quadruple::OP_ASSIGN, expr, "", node->value);
            break;
        }

        case NODE_RETURN_STATEMENT: {
            std::string expr = "";
            if (!node->children.empty()) {
                expr = generateExpression(node->children[0].get());
            }
            code.emplace_back(Quadruple::OP_RETURN, expr, "", "");
            break;
        }

        case NODE_IF_STATEMENT: {
            std::string condition = generateExpression(node->children[0].get());
            code.emplace_back(Quadruple::OP_IF, condition, "", "");
            if (node->children.size() > 1) {
                generateNode(node->children[1].get(), code);
            }
            if (node->children.size() > 2) {
                code.emplace_back(Quadruple::OP_ELSE, "", "", "");
                generateNode(node->children[2].get(), code);
            }
            break;
        }

        case NODE_WHILE_LOOP: {
            std::string condition = generateExpression(node->children[0].get());
            code.emplace_back(Quadruple::OP_WHILE, condition, "", "");
            if (node->children.size() > 1) {
                generateNode(node->children[1].get(), code);
            }
            break;
        }

        case NODE_CALL_EXPRESSION: {
            if (node->value == "printf") {
                std::string arg = node->children.empty() ? "" : generateExpression(node->children[0].get());
                code.emplace_back(Quadruple::OP_PRINTF, arg, "", "");
            } else if (node->value == "scanf") {
                std::string arg = node->children.empty() ? "" : generateExpression(node->children[0].get());
                code.emplace_back(Quadruple::OP_SCANF, arg, "", "");
            } else {
                std::string callName = node->value;
                std::string args = "";
                for (size_t i = 0; i < node->children.size(); i++) {
                    if (i > 0) args += ", ";
                    args += generateExpression(node->children[i].get());
                }
                code.emplace_back(Quadruple::OP_CALL, callName, args, callName);
            }
            break;
        }

        case NODE_BLOCK:
            for (auto& child : node->children) {
                generateNode(child.get(), code);
            }
            break;

        default:
            for (auto& child : node->children) {
                generateNode(child.get(), code);
            }
            break;
    }
}

std::vector<Quadruple> IntermediateCodeGenerator::generate(ASTNode* ast) {
    std::vector<Quadruple> code;
    if (ast) {
        generateNode(ast, code);
    }
    return code;
}