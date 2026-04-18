// PHASE 2: SYNTAX ANALYSIS (PARSER)
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

bool Parser::checkIdentifierFollowedByAssign() const {
    if (!check(Token::TOKEN_IDENTIFIER)) return false;
    if (current + 1 >= tokens.size()) return false;
    return tokens[current + 1].type == Token::TOKEN_ASSIGN;
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
    auto program = new ASTNode(NODE_PROGRAM, "", peek().line, peek().column);
    while (!isAtEnd()) {
        program->addChild(std::unique_ptr<ASTNode>(parseStatement()));
    }
    return program;
}

ASTNode* Parser::parseStatement() {
    if (match(Token::TOKEN_SEMICOLON)) {
        return new ASTNode(NODE_BLOCK, "", previous().line, previous().column); // Empty statement
    }

    if (match(Token::TOKEN_LBRACE)) {
        auto block = new ASTNode(NODE_BLOCK, "", previous().line, previous().column);
        while (!check(Token::TOKEN_RBRACE) && !isAtEnd()) {
            block->addChild(std::unique_ptr<ASTNode>(parseStatement()));
        }
        if (!match(Token::TOKEN_RBRACE)) error("Expected }");
        return block;
    }

    if (match(Token::TOKEN_INT) || match(Token::TOKEN_FLOAT) || match(Token::TOKEN_CHAR) || match(Token::TOKEN_VOID)) {
        Token type = previous();
        if (!match(Token::TOKEN_IDENTIFIER)) error("Expected identifier after type");
        Token name = previous();

        if (match(Token::TOKEN_LPAREN)) {
            // Function definition
            auto func = new ASTNode(NODE_FUNCTION, name.value, name.line, name.column);
            if (!match(Token::TOKEN_RPAREN)) {
                do {
                    if (match(Token::TOKEN_INT) || match(Token::TOKEN_FLOAT) || match(Token::TOKEN_CHAR)) {
                        if (!match(Token::TOKEN_IDENTIFIER)) error("Expected parameter name");
                        func->addChild(std::unique_ptr<ASTNode>(new ASTNode(NODE_PARAMETER, previous().value, previous().line, previous().column)));
                    }
                } while (match(Token::TOKEN_COMMA));
                if (!match(Token::TOKEN_RPAREN)) error("Expected ) after parameters");
            }
            if (!match(Token::TOKEN_LBRACE)) error("Expected { after function header");
            while (!check(Token::TOKEN_RBRACE) && !isAtEnd()) {
                func->addChild(std::unique_ptr<ASTNode>(parseStatement()));
            }
            if (!match(Token::TOKEN_RBRACE)) error("Expected } after function body");
            return func;
        } else {
            // Variable declaration
            auto decl = new ASTNode(NODE_VARIABLE_DECL, name.value, name.line, name.column);
            if (match(Token::TOKEN_ASSIGN)) {
                decl->addChild(std::unique_ptr<ASTNode>(parseExpression()));
            }
            if (!match(Token::TOKEN_SEMICOLON)) error("Expected ; after variable declaration");
            return decl;
        }
    } else if (match(Token::TOKEN_IDENTIFIER) || match(Token::TOKEN_PRINTF) || match(Token::TOKEN_SCANF)) {
        Token idTok = previous();
        auto id = idTok.value;
        if (match(Token::TOKEN_ASSIGN)) {
            auto assign = new ASTNode(NODE_ASSIGNMENT, id, idTok.line, idTok.column);
            assign->addChild(std::unique_ptr<ASTNode>(parseExpression()));
            if (!match(Token::TOKEN_SEMICOLON)) error("Expected ;");
            return assign;
        } else if (match(Token::TOKEN_LPAREN)) {
            // Function call
            auto call = new ASTNode(NODE_CALL_EXPRESSION, id, idTok.line, idTok.column);
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
        auto ret = new ASTNode(NODE_RETURN_STATEMENT, "", previous().line, previous().column);
        ret->addChild(std::unique_ptr<ASTNode>(parseExpression()));
        if (!match(Token::TOKEN_SEMICOLON)) error("Expected ;");
        return ret;
    } else if (match(Token::TOKEN_IF)) {
        auto ifStmt = new ASTNode(NODE_IF_STATEMENT, "", previous().line, previous().column);
        if (!match(Token::TOKEN_LPAREN)) error("Expected (");
        ifStmt->addChild(std::unique_ptr<ASTNode>(parseExpression()));
        if (!match(Token::TOKEN_RPAREN)) error("Expected )");
        
        // Support both block { } and single statement
        if (match(Token::TOKEN_LBRACE)) {
            while (!check(Token::TOKEN_RBRACE) && !isAtEnd()) {
                ifStmt->addChild(std::unique_ptr<ASTNode>(parseStatement()));
            }
            if (!match(Token::TOKEN_RBRACE)) error("Expected }");
        } else {
            // Single statement - parse one statement without braces
            ifStmt->addChild(std::unique_ptr<ASTNode>(parseStatement()));
        }
        
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
        auto whileStmt = new ASTNode(NODE_WHILE_LOOP, "", previous().line, previous().column);
        if (!match(Token::TOKEN_LPAREN)) error("Expected ( after while");
        whileStmt->addChild(std::unique_ptr<ASTNode>(parseExpression()));
        if (!match(Token::TOKEN_RPAREN)) error("Expected ) after while condition");
        
        // Support both block { } and single statement
        if (match(Token::TOKEN_LBRACE)) {
            while (!check(Token::TOKEN_RBRACE) && !isAtEnd()) {
                whileStmt->addChild(std::unique_ptr<ASTNode>(parseStatement()));
            }
            if (!match(Token::TOKEN_RBRACE)) error("Expected } after while body");
        } else {
            // Single statement
            whileStmt->addChild(std::unique_ptr<ASTNode>(parseStatement()));
        }
        return whileStmt;
    } else if (match(Token::TOKEN_FOR)) {
        auto forStmt = new ASTNode(NODE_FOR_LOOP, "", previous().line, previous().column);
        if (!match(Token::TOKEN_LPAREN)) error("Expected ( after for");
        
        // Child 0: initializer (empty block if omitted)
        if (match(Token::TOKEN_INT) || match(Token::TOKEN_FLOAT) || match(Token::TOKEN_CHAR)) {
            if (!match(Token::TOKEN_IDENTIFIER)) error("Expected identifier in for loop");
            Token name = previous();
            auto decl = new ASTNode(NODE_VARIABLE_DECL, name.value, name.line, name.column);
            if (match(Token::TOKEN_ASSIGN)) {
                decl->addChild(std::unique_ptr<ASTNode>(parseExpression()));
            }
            forStmt->addChild(std::unique_ptr<ASTNode>(decl));
        } else if (!check(Token::TOKEN_SEMICOLON)) {
            if (checkIdentifierFollowedByAssign()) {
                if (!match(Token::TOKEN_IDENTIFIER)) error("Expected identifier");
                Token idTok = previous();
                match(Token::TOKEN_ASSIGN);
                auto assign = new ASTNode(NODE_ASSIGNMENT, idTok.value, idTok.line, idTok.column);
                assign->addChild(std::unique_ptr<ASTNode>(parseExpression()));
                forStmt->addChild(std::unique_ptr<ASTNode>(assign));
            } else {
                forStmt->addChild(std::unique_ptr<ASTNode>(parseExpression()));
            }
        } else {
            forStmt->addChild(std::unique_ptr<ASTNode>(new ASTNode(NODE_BLOCK, "", previous().line, previous().column)));
        }
        
        if (!match(Token::TOKEN_SEMICOLON)) error("Expected ; after for init");
        
        // Child 1: condition (default true)
        if (!check(Token::TOKEN_SEMICOLON)) {
            forStmt->addChild(std::unique_ptr<ASTNode>(parseExpression()));
        } else {
            forStmt->addChild(std::unique_ptr<ASTNode>(new ASTNode(NODE_NUMBER_LITERAL, "1", previous().line, previous().column)));
        }
        if (!match(Token::TOKEN_SEMICOLON)) error("Expected ; after for condition");
        
        // Child 2: increment (empty block if omitted)
        if (!check(Token::TOKEN_RPAREN)) {
            if (checkIdentifierFollowedByAssign()) {
                if (!match(Token::TOKEN_IDENTIFIER)) error("Expected identifier");
                Token idTok = previous();
                match(Token::TOKEN_ASSIGN);
                auto assign = new ASTNode(NODE_ASSIGNMENT, idTok.value, idTok.line, idTok.column);
                assign->addChild(std::unique_ptr<ASTNode>(parseExpression()));
                forStmt->addChild(std::unique_ptr<ASTNode>(assign));
            } else {
                forStmt->addChild(std::unique_ptr<ASTNode>(parseExpression()));
            }
        } else {
            forStmt->addChild(std::unique_ptr<ASTNode>(new ASTNode(NODE_BLOCK, "", previous().line, previous().column)));
        }
        
        if (!match(Token::TOKEN_RPAREN)) error("Expected ) after for header");
        
        // Remaining children: body statement(s)
        if (match(Token::TOKEN_LBRACE)) {
            while (!check(Token::TOKEN_RBRACE) && !isAtEnd()) {
                forStmt->addChild(std::unique_ptr<ASTNode>(parseStatement()));
            }
            if (!match(Token::TOKEN_RBRACE)) error("Expected } after for body");
        } else {
            forStmt->addChild(std::unique_ptr<ASTNode>(parseStatement()));
        }
        return forStmt;
    } else if (match(Token::TOKEN_SWITCH)) {
        auto switchStmt = new ASTNode(NODE_SWITCH_STATEMENT, "", previous().line, previous().column);
        if (!match(Token::TOKEN_LPAREN)) error("Expected ( after switch");
        switchStmt->addChild(std::unique_ptr<ASTNode>(parseExpression()));
        if (!match(Token::TOKEN_RPAREN)) error("Expected ) after switch expression");
        
        if (!match(Token::TOKEN_LBRACE)) error("Expected { after switch");
        
        // Parse cases
        while (!check(Token::TOKEN_RBRACE) && !isAtEnd()) {
            if (match(Token::TOKEN_CASE)) {
                auto caseNode = new ASTNode(NODE_CASE_STATEMENT, "", previous().line, previous().column);
                // Parse case value
                if (match(Token::TOKEN_NUMBER)) {
                    caseNode->addChild(std::unique_ptr<ASTNode>(new ASTNode(NODE_NUMBER_LITERAL, previous().value, previous().line, previous().column)));
                } else if (match(Token::TOKEN_IDENTIFIER)) {
                    caseNode->addChild(std::unique_ptr<ASTNode>(new ASTNode(NODE_IDENTIFIER, previous().value, previous().line, previous().column)));
                }
                if (!match(Token::TOKEN_COLON)) error("Expected : after case");
                
                // Parse case body
                while (!check(Token::TOKEN_CASE) && !check(Token::TOKEN_DEFAULT) && !check(Token::TOKEN_RBRACE) && !isAtEnd()) {
                    caseNode->addChild(std::unique_ptr<ASTNode>(parseStatement()));
                }
                switchStmt->addChild(std::unique_ptr<ASTNode>(caseNode));
            } else if (match(Token::TOKEN_DEFAULT)) {
                auto defaultNode = new ASTNode(NODE_CASE_STATEMENT, "default", previous().line, previous().column);
                if (!match(Token::TOKEN_COLON)) error("Expected : after default");
                
                while (!check(Token::TOKEN_CASE) && !check(Token::TOKEN_RBRACE) && !isAtEnd()) {
                    defaultNode->addChild(std::unique_ptr<ASTNode>(parseStatement()));
                }
                switchStmt->addChild(std::unique_ptr<ASTNode>(defaultNode));
            } else if (match(Token::TOKEN_BREAK)) {
                auto breakNode = new ASTNode(NODE_BREAK_STATEMENT, "", previous().line, previous().column);
                if (!match(Token::TOKEN_SEMICOLON)) error("Expected ; after break");
                switchStmt->addChild(std::unique_ptr<ASTNode>(breakNode));
            } else {
                // Skip other statements
                advance();
            }
        }
        
        if (!match(Token::TOKEN_RBRACE)) error("Expected } after switch body");
        return switchStmt;
    } else if (match(Token::TOKEN_BREAK)) {
        auto breakNode = new ASTNode(NODE_BREAK_STATEMENT, "", previous().line, previous().column);
        if (!match(Token::TOKEN_SEMICOLON)) error("Expected ; after break");
        return breakNode;
    } else if (match(Token::TOKEN_CONTINUE)) {
        auto continueNode = new ASTNode(NODE_CONTINUE_STATEMENT, "", previous().line, previous().column);
        if (!match(Token::TOKEN_SEMICOLON)) error("Expected ; after continue");
        return continueNode;
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
        Token opTok = previous();
        std::string op = opTok.value;
        ASTNode* right = parseComparison();
        auto expr = new ASTNode(NODE_BINARY_OP, op, opTok.line, opTok.column);
        expr->addChild(std::unique_ptr<ASTNode>(node));
        expr->addChild(std::unique_ptr<ASTNode>(right));
        node = expr;
    }

    return node;
}

ASTNode* Parser::parseComparison() {
    ASTNode* node = parseTerm();

    while (match(Token::TOKEN_LESS) || match(Token::TOKEN_LESS_EQUAL) || match(Token::TOKEN_GREATER) || match(Token::TOKEN_GREATER_EQUAL)) {
        Token opTok = previous();
        std::string op = opTok.value;
        ASTNode* right = parseTerm();
        auto expr = new ASTNode(NODE_BINARY_OP, op, opTok.line, opTok.column);
        expr->addChild(std::unique_ptr<ASTNode>(node));
        expr->addChild(std::unique_ptr<ASTNode>(right));
        node = expr;
    }

    return node;
}

ASTNode* Parser::parseTerm() {
    ASTNode* node = parseFactor();

    while (match(Token::TOKEN_PLUS) || match(Token::TOKEN_MINUS)) {
        Token opTok = previous();
        std::string op = opTok.value;
        ASTNode* right = parseFactor();
        auto expr = new ASTNode(NODE_BINARY_OP, op, opTok.line, opTok.column);
        expr->addChild(std::unique_ptr<ASTNode>(node));
        expr->addChild(std::unique_ptr<ASTNode>(right));
        node = expr;
    }

    return node;
}

ASTNode* Parser::parseFactor() {
    ASTNode* node = parseUnary();

    while (match(Token::TOKEN_MULTIPLY) || match(Token::TOKEN_DIVIDE) || match(Token::TOKEN_MODULO)) {
        Token opTok = previous();
        std::string op = opTok.value;
        ASTNode* right = parseUnary();
        auto expr = new ASTNode(NODE_BINARY_OP, op, opTok.line, opTok.column);
        expr->addChild(std::unique_ptr<ASTNode>(node));
        expr->addChild(std::unique_ptr<ASTNode>(right));
        node = expr;
    }

    return node;
}

ASTNode* Parser::parseUnary() {
    if (match(Token::TOKEN_AMPERSAND)) {
        Token opTok = previous();
        ASTNode* right = parseUnary();
        auto expr = new ASTNode(NODE_UNARY_OP, "&", opTok.line, opTok.column);
        expr->addChild(std::unique_ptr<ASTNode>(right));
        return expr;
    }
    if (match(Token::TOKEN_NOT) || match(Token::TOKEN_MINUS)) {
        Token opTok = previous();
        std::string op = opTok.value;
        ASTNode* right = parseUnary();
        auto expr = new ASTNode(NODE_UNARY_OP, op, opTok.line, opTok.column);
        expr->addChild(std::unique_ptr<ASTNode>(right));
        return expr;
    }
    return parsePrimary();
}

ASTNode* Parser::parsePrimary() {
    if (match(Token::TOKEN_NUMBER)) {
        return new ASTNode(NODE_NUMBER_LITERAL, previous().value, previous().line, previous().column);
    } else if (match(Token::TOKEN_STRING)) {
        return new ASTNode(NODE_STRING_LITERAL, previous().value, previous().line, previous().column);
    } else if (match(Token::TOKEN_IDENTIFIER) || match(Token::TOKEN_PRINTF) || match(Token::TOKEN_SCANF)) {
        Token nameTok = previous();
        std::string name = nameTok.value;
        if (match(Token::TOKEN_LPAREN)) {
            auto call = new ASTNode(NODE_CALL_EXPRESSION, name, nameTok.line, nameTok.column);
            if (!match(Token::TOKEN_RPAREN)) {
                do {
                    call->addChild(std::unique_ptr<ASTNode>(parseExpression()));
                } while (match(Token::TOKEN_COMMA));
                if (!match(Token::TOKEN_RPAREN)) error("Expected )");
            }
            return call;
        }
        return new ASTNode(NODE_IDENTIFIER, name, nameTok.line, nameTok.column);
    } else if (match(Token::TOKEN_LPAREN)) {
        ASTNode* expr = parseExpression();
        if (!match(Token::TOKEN_RPAREN)) error("Expected )");
        return expr;
    }
    error("Expected primary expression");
    return nullptr;
}