// PHASE 4: INTERMEDIATE CODE GENERATION (QUADRUPLES)
#include "../include/intermediate_code.h"
#include <sstream>
#include <map>

static int labelCounter = 0;

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
        case OP_ELSE_IF: return "ELSE_IF";
        case OP_ELSE: return "ELSE";
        case OP_WHILE: return "WHILE";
        case OP_FOR: return "FOR";
        case OP_SWITCH: return "SWITCH";
        case OP_CASE: return "CASE";
        case OP_BREAK: return "BREAK";
        case OP_CONTINUE: return "CONTINUE";
        case OP_EXIT: return "EXIT";
        case OP_PRINTF: return "PRINTF";
        case OP_SCANF: return "SCANF";
        case OP_RETURN: return "RETURN";
        case OP_FUNC_START: return "FUNC_START";
        case OP_FUNC_END: return "FUNC_END";
        case OP_BLOCK_END: return "BLOCK_END";
        default: return "UNKNOWN";
    }
}

static std::string newTemp() {
    static int tempCounter = 0;
    return "_t" + std::to_string(tempCounter++);
}

static std::string generateExpression(ASTNode* node, std::vector<Quadruple>& code) {
    if (!node) return "";
    
    switch (node->type) {
        case NODE_NUMBER_LITERAL:
        case NODE_STRING_LITERAL:
            return node->value;
            
        case NODE_IDENTIFIER:
            return node->value;
            
        case NODE_UNARY_OP: {
            std::string operand = generateExpression(node->children[0].get(), code);
            std::string temp = newTemp();
            if (node->value == "-") {
                code.emplace_back(Quadruple::OP_SUB, "0", operand, temp);
            } else if (node->value == "!") {
                code.emplace_back(Quadruple::OP_NOT, operand, "", temp);
            } else {
                code.emplace_back(Quadruple::OP_ASSIGN, operand, "", temp);
            }
            return temp;
        }
        
        case NODE_BINARY_OP: {
            std::string left = generateExpression(node->children[0].get(), code);
            std::string right = generateExpression(node->children[1].get(), code);
            std::string temp = newTemp();
            
            Quadruple::Op op;
            if (node->value == "+") op = Quadruple::OP_ADD;
            else if (node->value == "-") op = Quadruple::OP_SUB;
            else if (node->value == "*") op = Quadruple::OP_MUL;
            else if (node->value == "/") op = Quadruple::OP_DIV;
            else if (node->value == "%") op = Quadruple::OP_MOD;
            else if (node->value == "<") op = Quadruple::OP_LT;
            else if (node->value == ">") op = Quadruple::OP_GT;
            else if (node->value == "<=") op = Quadruple::OP_LE;
            else if (node->value == ">=") op = Quadruple::OP_GE;
            else if (node->value == "==") op = Quadruple::OP_EQ;
            else if (node->value == "!=") op = Quadruple::OP_NE;
            else if (node->value == "&&") op = Quadruple::OP_AND;
            else if (node->value == "||") op = Quadruple::OP_OR;
            else op = Quadruple::OP_ASSIGN;
            
            code.emplace_back(op, left, right, temp);
            return temp;
        }
        
        case NODE_CALL_EXPRESSION: {
            if (node->value == "printf") {
                std::string fmt = node->children.empty() ? "" : generateExpression(node->children[0].get(), code);
                std::string rest = "";
                for (size_t i = 1; i < node->children.size(); i++) {
                    if (i > 1) rest += ", ";
                    rest += generateExpression(node->children[i].get(), code);
                }
                code.emplace_back(Quadruple::OP_PRINTF, fmt, rest, "");
                return "";
            } else if (node->value == "scanf") {
                std::string varName;
                if (node->children.size() >= 2) {
                    ASTNode* addr = node->children[1].get();
                    if (addr && addr->type == NODE_UNARY_OP && addr->value == "&" && !addr->children.empty()) {
                        varName = addr->children[0]->value;
                    } else if (addr && addr->type == NODE_IDENTIFIER) {
                        varName = addr->value;
                    }
                }
                code.emplace_back(Quadruple::OP_SCANF, varName, "", "");
                return "";
            } else if (node->value == "exit") {
                std::string arg = node->children.empty() ? "0" : generateExpression(node->children[0].get(), code);
                code.emplace_back(Quadruple::OP_EXIT, arg, "", "");
                return "";
            } else {
                std::string args = "";
                for (size_t i = 0; i < node->children.size(); i++) {
                    if (i > 0) args += ", ";
                    args += generateExpression(node->children[i].get(), code);
                }
                std::string temp = newTemp();
                code.emplace_back(Quadruple::OP_CALL, node->value, args, temp);
                return temp;
            }
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
            // Build parameter string
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
            if (!node->children.empty()) {
                std::string initValue = generateExpression(node->children[0].get(), code);
                code.emplace_back(Quadruple::OP_ASSIGN, initValue, "", node->value);
            }
            break;
        }

        case NODE_ASSIGNMENT: {
            std::string expr = generateExpression(node->children[0].get(), code);
            code.emplace_back(Quadruple::OP_ASSIGN, expr, "", node->value);
            break;
        }

        case NODE_RETURN_STATEMENT: {
            std::string expr = "";
            if (!node->children.empty()) {
                expr = generateExpression(node->children[0].get(), code);
            }
            code.emplace_back(Quadruple::OP_RETURN, expr, "", "");
            break;
        }

        case NODE_IF_STATEMENT: {
            std::string condition = generateExpression(node->children[0].get(), code);
            code.emplace_back(Quadruple::OP_IF, condition, "", "");
            if (node->children.size() > 1) {
                generateNode(node->children[1].get(), code);
            }
            if (node->children.size() > 2) {
                code.emplace_back(Quadruple::OP_ELSE, "", "", "");
                generateNode(node->children[2].get(), code);
            }
            code.emplace_back(Quadruple::OP_BLOCK_END, "", "", "");
            break;
        }

        case NODE_WHILE_LOOP: {
            std::string condition = generateExpression(node->children[0].get(), code);
            code.emplace_back(Quadruple::OP_WHILE, condition, "", "");
            for (size_t i = 1; i < node->children.size(); i++) {
                generateNode(node->children[i].get(), code);
            }
            code.emplace_back(Quadruple::OP_BLOCK_END, "", "", "");
            break;
        }

        case NODE_FOR_LOOP: {
            if (node->children.size() > 0 && node->children[0]) {
                generateNode(node->children[0].get(), code);
            }
            int endLabel = labelCounter++;
            code.emplace_back(Quadruple::OP_WHILE, "1", "", "");
            if (node->children.size() > 1 && node->children[1]) {
                std::string condition = generateExpression(node->children[1].get(), code);
                code.emplace_back(Quadruple::OP_JMP_FALSE, condition, "", "L" + std::to_string(endLabel));
            }
            for (size_t i = 3; i < node->children.size(); i++) {
                generateNode(node->children[i].get(), code);
            }
            if (node->children.size() > 2 && node->children[2]) {
                generateNode(node->children[2].get(), code);
            }
            code.emplace_back(Quadruple::OP_BLOCK_END, "", "", "");
            code.emplace_back(Quadruple::OP_LABEL, "", "", "L" + std::to_string(endLabel));
            break;
        }

        case NODE_SWITCH_STATEMENT: {
            // Generate switch expression
            std::string switchVar = "";
            if (!node->children.empty()) {
                switchVar = generateExpression(node->children[0].get(), code);
            }
            
            // Generate if-else chain for each case
            bool firstCase = true;
            for (size_t i = 1; i < node->children.size(); i++) {
                auto& caseNode = node->children[i];
                if (caseNode->type == NODE_CASE_STATEMENT) {
                    std::string caseValue;
                    
                    if (caseNode->children.empty() || caseNode->value == "default") {
                        // Default case - use OP_ELSE
                        code.emplace_back(Quadruple::OP_ELSE, "", "", "");
                    } else {
                        caseValue = caseNode->children[0]->value;
                        
                        // Generate comparison: if (switchVar == caseValue)
                        std::string temp = newTemp();
                        code.emplace_back(Quadruple::OP_EQ, switchVar, caseValue, temp);
                        
                        if (firstCase) {
                            code.emplace_back(Quadruple::OP_IF, temp, "", "");
                        } else {
                            code.emplace_back(Quadruple::OP_ELSE_IF, temp, "", "");
                        }
                        firstCase = false;
                    }
                    
                    // Generate case body (skip first child which is the case value)
                    for (size_t j = 1; j < caseNode->children.size(); j++) {
                        generateNode(caseNode->children[j].get(), code);
                    }
                } else if (caseNode->type == NODE_BREAK_STATEMENT) {
                    // Break is handled by ending the case block
                } else {
                    generateNode(caseNode.get(), code);
                }
            }
            code.emplace_back(Quadruple::OP_BLOCK_END, "", "", "");
            break;
        }

        case NODE_BREAK_STATEMENT: {
            // Break will be handled by parent switch/while/for
            break;
        }

        case NODE_CONTINUE_STATEMENT: {
            // Continue will be handled by parent while/for
            break;
        }

        case NODE_CALL_EXPRESSION: {
            generateExpression(node, code);
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
    labelCounter = 0;
    if (ast) {
        generateNode(ast, code);
    }
    return code;
}