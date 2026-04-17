#include "../include/parser.h"
#include <iostream>

Parser::Parser(const std::vector<Token>& toks) : tokens(toks), current(0) {}

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return current >= tokens.size() || peek().type == Token::TOKEN_EOF;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::check(Token::Type type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(Token::Type type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

void Parser::error(const std::string& message) {
    std::cerr << "Parse error at line " << peek().line << ": " << message << std::endl;
    throw std::runtime_error(message);
}

ASTNode* Parser::parse() {
    try {
        return parseProgram();
    } catch (const std::exception& e) {
        return nullptr;
    }
}

ASTNode* Parser::parseProgram() {
    auto program = new ASTNode(NODE_PROGRAM);
    while (!isAtEnd()) {
        program->addChild(std::unique_ptr<ASTNode>(parseStatement()));
    }
    return program;
}

ASTNode* Parser::parseStatement() {
    if (match(Token::TOKEN_INT) || match(Token::TOKEN_FLOAT) || match(Token::TOKEN_CHAR) || match(Token::TOKEN_VOID)) {
        // Variable declaration or function
        auto type = previous().value;
        if (check(Token::TOKEN_IDENTIFIER)) {
            auto id = advance().value;
            if (match(Token::TOKEN_LPAREN)) {
                // Function
                auto func = new ASTNode(NODE_FUNCTION, id);
                // Support parameters
                if (!check(Token::TOKEN_RPAREN)) {
                    do {
                        // Skip type (int, float, etc.)
                        if (!(match(Token::TOKEN_INT) || match(Token::TOKEN_FLOAT) || match(Token::TOKEN_CHAR) || match(Token::TOKEN_VOID))) {
                            error("Expected parameter type");
                        }
                        if (check(Token::TOKEN_IDENTIFIER)) {
                            auto paramId = advance().value;
                            func->addChild(std::unique_ptr<ASTNode>(new ASTNode(NODE_PARAMETER, paramId)));
                        } else {
                            error("Expected parameter identifier");
                        }
                    } while (match(Token::TOKEN_COMMA));
                }
                if (!match(Token::TOKEN_RPAREN)) error("Expected ) after parameters");
                if (!match(Token::TOKEN_LBRACE)) error("Expected {");
                while (!check(Token::TOKEN_RBRACE) && !isAtEnd()) {
                    func->addChild(std::unique_ptr<ASTNode>(parseStatement()));
                }
                if (!match(Token::TOKEN_RBRACE)) error("Expected }");
                return func;
            } else {
                // Variable
                auto var = new ASTNode(NODE_VARIABLE_DECL, id);
                if (match(Token::TOKEN_ASSIGN)) {
                    var->addChild(std::unique_ptr<ASTNode>(parseExpression()));
                }
                if (!match(Token::TOKEN_SEMICOLON)) error("Expected ;");
                return var;
            }
        }
    } else if (match(Token::TOKEN_IDENTIFIER) || match(Token::TOKEN_PRINTF) || match(Token::TOKEN_SCANF)) {
        auto id = previous().value;
        if (match(Token::TOKEN_ASSIGN)) {
            auto assign = new ASTNode(NODE_ASSIGNMENT, id);
            assign->addChild(std::unique_ptr<ASTNode>(parseExpression()));
            if (!match(Token::TOKEN_SEMICOLON)) error("Expected ;");
            return assign;
        } else if (match(Token::TOKEN_LPAREN)) {
            // Function call
            auto call = new ASTNode(NODE_CALL_EXPRESSION, id);
            if (!match(Token::TOKEN_RPAREN)) {
                do {
                    call->addChild(std::unique_ptr<ASTNode>(parseExpression()));
                } while (match(Token::TOKEN_COMMA));
                if (!match(Token::TOKEN_RPAREN)) error("Expected )");
            }
            if (!match(Token::TOKEN_SEMICOLON)) error("Expected ;");
            return call;
        }
    } else if (match(Token::TOKEN_RETURN)) {
        auto ret = new ASTNode(NODE_RETURN_STATEMENT);
        ret->addChild(std::unique_ptr<ASTNode>(parseExpression()));
        if (!match(Token::TOKEN_SEMICOLON)) error("Expected ;");
        return ret;
    } else if (match(Token::TOKEN_IF)) {
        auto ifStmt = new ASTNode(NODE_IF_STATEMENT);
        if (!match(Token::TOKEN_LPAREN)) error("Expected (");
        ifStmt->addChild(std::unique_ptr<ASTNode>(parseExpression()));
        if (!match(Token::TOKEN_RPAREN)) error("Expected )");
        if (!match(Token::TOKEN_LBRACE)) error("Expected {");
        while (!check(Token::TOKEN_RBRACE) && !isAtEnd()) {
            ifStmt->addChild(std::unique_ptr<ASTNode>(parseStatement()));
        }
        if (!match(Token::TOKEN_RBRACE)) error("Expected }");
        
        // Support for optional ELSE
        if (match(Token::TOKEN_ELSE)) {
            if (match(Token::TOKEN_LBRACE)) {
                while (!check(Token::TOKEN_RBRACE) && !isAtEnd()) {
                    ifStmt->addChild(std::unique_ptr<ASTNode>(parseStatement()));
                }
                if (!match(Token::TOKEN_RBRACE)) error("Expected } after else");
            } else {
                // Single statement else
                ifStmt->addChild(std::unique_ptr<ASTNode>(parseStatement()));
            }
        }
        return ifStmt;
    } else if (match(Token::TOKEN_WHILE)) {
        auto whileStmt = new ASTNode(NODE_WHILE_STATEMENT);
        if (!match(Token::TOKEN_LPAREN)) error("Expected ( after while");
        whileStmt->addChild(std::unique_ptr<ASTNode>(parseExpression()));
        if (!match(Token::TOKEN_RPAREN)) error("Expected ) after while condition");
        if (!match(Token::TOKEN_LBRACE)) error("Expected { after while condition");
        while (!check(Token::TOKEN_RBRACE) && !isAtEnd()) {
            whileStmt->addChild(std::unique_ptr<ASTNode>(parseStatement()));
        }
        if (!match(Token::TOKEN_RBRACE)) error("Expected } after while body");
        return whileStmt;
    } else if (match(Token::TOKEN_FOR)) {
        auto forStmt = new ASTNode(NODE_FOR_STATEMENT);
        if (!match(Token::TOKEN_LPAREN)) error("Expected ( after for");
        
        // Initializer
        forStmt->addChild(std::unique_ptr<ASTNode>(parseStatement()));
        
        // Condition
        forStmt->addChild(std::unique_ptr<ASTNode>(parseExpression()));
        if (!match(Token::TOKEN_SEMICOLON)) error("Expected ; after for condition");
        
        // Increment
        forStmt->addChild(std::unique_ptr<ASTNode>(parseExpression()));
        
        if (!match(Token::TOKEN_RPAREN)) error("Expected ) after for header");
        if (!match(Token::TOKEN_LBRACE)) error("Expected { after for header");
        
        while (!check(Token::TOKEN_RBRACE) && !isAtEnd()) {
            forStmt->addChild(std::unique_ptr<ASTNode>(parseStatement()));
        }
        if (!match(Token::TOKEN_RBRACE)) error("Expected } after for body");
        return forStmt;
    }
    error("Unexpected token");
    return nullptr;
}

ASTNode* Parser::parseExpression() {
    return parseEquality();
}

ASTNode* Parser::parseEquality() {
    ASTNode* node = parseComparison();

    while (match(Token::TOKEN_EQUALS) || match(Token::TOKEN_NOT_EQUALS)) {
        std::string op = previous().value;
        ASTNode* right = parseComparison();
        auto expr = new ASTNode(NODE_BINARY_OP, op);
        expr->addChild(std::unique_ptr<ASTNode>(node));
        expr->addChild(std::unique_ptr<ASTNode>(right));
        node = expr;
    }

    return node;
}

ASTNode* Parser::parseComparison() {
    ASTNode* node = parseTerm();

    while (match(Token::TOKEN_LESS) || match(Token::TOKEN_LESS_EQUAL) || match(Token::TOKEN_GREATER) || match(Token::TOKEN_GREATER_EQUAL)) {
        std::string op = previous().value;
        ASTNode* right = parseTerm();
        auto expr = new ASTNode(NODE_BINARY_OP, op);
        expr->addChild(std::unique_ptr<ASTNode>(node));
        expr->addChild(std::unique_ptr<ASTNode>(right));
        node = expr;
    }

    return node;
}

ASTNode* Parser::parseTerm() {
    ASTNode* node = parseFactor();

    while (match(Token::TOKEN_PLUS) || match(Token::TOKEN_MINUS)) {
        std::string op = previous().value;
        ASTNode* right = parseFactor();
        auto expr = new ASTNode(NODE_BINARY_OP, op);
        expr->addChild(std::unique_ptr<ASTNode>(node));
        expr->addChild(std::unique_ptr<ASTNode>(right));
        node = expr;
    }

    return node;
}

ASTNode* Parser::parseFactor() {
    ASTNode* node = parseUnary();

    while (match(Token::TOKEN_MULTIPLY) || match(Token::TOKEN_DIVIDE) || match(Token::TOKEN_MODULO)) {
        std::string op = previous().value;
        ASTNode* right = parseUnary();
        auto expr = new ASTNode(NODE_BINARY_OP, op);
        expr->addChild(std::unique_ptr<ASTNode>(node));
        expr->addChild(std::unique_ptr<ASTNode>(right));
        node = expr;
    }

    return node;
}

ASTNode* Parser::parseUnary() {
    if (match(Token::TOKEN_NOT) || match(Token::TOKEN_MINUS)) {
        std::string op = previous().value;
        ASTNode* right = parseUnary();
        auto expr = new ASTNode(NODE_UNARY_OP, op);
        expr->addChild(std::unique_ptr<ASTNode>(right));
        return expr;
    }
    return parsePrimary();
}

ASTNode* Parser::parsePrimary() {
    if (match(Token::TOKEN_NUMBER)) {
        return new ASTNode(NODE_NUMBER_LITERAL, previous().value);
    } else if (match(Token::TOKEN_STRING)) {
        return new ASTNode(NODE_STRING_LITERAL, previous().value);
    } else if (match(Token::TOKEN_IDENTIFIER) || match(Token::TOKEN_PRINTF) || match(Token::TOKEN_SCANF)) {
        std::string name = previous().value;
        if (match(Token::TOKEN_LPAREN)) {
            auto call = new ASTNode(NODE_CALL_EXPRESSION, name);
            if (!match(Token::TOKEN_RPAREN)) {
                do {
                    call->addChild(std::unique_ptr<ASTNode>(parseExpression()));
                } while (match(Token::TOKEN_COMMA));
                if (!match(Token::TOKEN_RPAREN)) error("Expected )");
            }
            return call;
        }
        return new ASTNode(NODE_IDENTIFIER, name);
    } else if (match(Token::TOKEN_LPAREN)) {
        ASTNode* expr = parseExpression();
        if (!match(Token::TOKEN_RPAREN)) error("Expected )");
        return expr;
    }
    error("Expected primary expression");
    return nullptr;
}