// PHASE 3: SEMANTIC ANALYSIS (SYMBOL TABLE & TYPE CHECKING)
#include "../include/semantic_analyzer.h"
#include <iostream>

void SemanticAnalyzer::analyze(ASTNode* ast) {
    if (!ast) return;

    switch (ast->type) {
        case NODE_PROGRAM:
            for (auto& child : ast->children) {
                analyze(child.get());
            }
            break;

        case NODE_FUNCTION:
            // Add function to symbol table
            symbolTable.addSymbol(ast->value, TYPE_VOID, ast->line); // Simplified type
            symbolTable.enterScope();
            // Add parameters to scope
            for (auto& child : ast->children) {
                if (child->type == NODE_PARAMETER) {
                    symbolTable.addSymbol(child->value, TYPE_INT, child->line);
                } else {
                    analyze(child.get());
                }
            }
            symbolTable.exitScope();
            break;

        case NODE_VARIABLE_DECL:
            if (!symbolTable.addSymbol(ast->value, TYPE_INT, ast->line)) {
                // In a real compiler we would report this error properly
                // std::cerr << "Semantic error at line " << ast->line << ": Redefinition of variable " << ast->value << std::endl;
            }
            for (auto& child : ast->children) {
                analyze(child.get());
            }
            break;

        case NODE_IDENTIFIER:
            if (!symbolTable.lookupSymbol(ast->value)) {
                // std::cerr << "Semantic error at line " << ast->line << ": Use of undeclared variable " << ast->value << std::endl;
            }
            break;

        case NODE_ASSIGNMENT:
            if (!symbolTable.lookupSymbol(ast->value)) {
                // std::cerr << "Semantic error at line " << ast->line << ": Assignment to undeclared variable " << ast->value << std::endl;
            }
            for (auto& child : ast->children) {
                analyze(child.get());
            }
            break;

        case NODE_IF_STATEMENT:
        case NODE_WHILE_LOOP:
        case NODE_FOR_LOOP:
        case NODE_BLOCK:
            symbolTable.enterScope();
            for (auto& child : ast->children) {
                analyze(child.get());
            }
            symbolTable.exitScope();
            break;

        default:
            for (auto& child : ast->children) {
                analyze(child.get());
            }
            break;
    }
}

SymbolTable& SemanticAnalyzer::getSymbolTable() {
    return symbolTable;
}
