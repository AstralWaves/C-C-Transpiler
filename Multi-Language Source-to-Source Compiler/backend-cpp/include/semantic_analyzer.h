#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include "symbol_table.h"

class SemanticAnalyzer {
private:
    SymbolTable symbolTable;
    
public:
    void analyze(ASTNode* ast);
    SymbolTable& getSymbolTable();
};

#endif