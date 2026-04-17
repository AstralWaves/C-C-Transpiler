#include "../include/semantic_analyzer.h"

void SemanticAnalyzer::analyze(ASTNode* ast) {
    // Basic semantic analysis
    // For now, just traverse and check
    if (!ast) return;
    // Implement traversal
}

SymbolTable& SemanticAnalyzer::getSymbolTable() {
    return symbolTable;
}