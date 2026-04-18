#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "lexer.h"
#include "ast.h"

class Parser {
private:
    std::vector<Token> tokens;
    size_t current;
    
    Token peek() const;
    Token previous() const;
    bool isAtEnd() const;
    Token advance();
    bool check(Token::Type type) const;
    bool match(Token::Type type);
    bool checkIdentifierFollowedByAssign() const;
    void error(const std::string& message);
    
    ASTNode* parseProgram();
    ASTNode* parseStatement();
    ASTNode* parseExpression();
    ASTNode* parseEquality();
    ASTNode* parseComparison();
    ASTNode* parseTerm();
    ASTNode* parseFactor();
    ASTNode* parseUnary();
    ASTNode* parsePrimary();
    
public:
    Parser(const std::vector<Token>& toks);
    ASTNode* parse();
};

#endif