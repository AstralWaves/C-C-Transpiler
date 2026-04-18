// PHASE 1: LEXICAL ANALYSIS (TOKENIZER)
#include "../include/lexer.h"
#include <cctype>
#include <iostream>

Lexer::Lexer(const std::string& src) 
    : source(src), position(0), line(1), column(1) {
    
    // Initialize keywords
    keywords["int"] = Token::TOKEN_INT;
    keywords["float"] = Token::TOKEN_FLOAT;
    keywords["char"] = Token::TOKEN_CHAR;
    keywords["void"] = Token::TOKEN_VOID;
    keywords["if"] = Token::TOKEN_IF;
    keywords["else"] = Token::TOKEN_ELSE;
    keywords["while"] = Token::TOKEN_WHILE;
    keywords["for"] = Token::TOKEN_FOR;
    keywords["return"] = Token::TOKEN_RETURN;
    keywords["break"] = Token::TOKEN_BREAK;
    keywords["continue"] = Token::TOKEN_CONTINUE;
    keywords["switch"] = Token::TOKEN_SWITCH;
    keywords["case"] = Token::TOKEN_CASE;
    keywords["default"] = Token::TOKEN_DEFAULT;
    keywords["include"] = Token::TOKEN_INCLUDE;
    keywords["printf"] = Token::TOKEN_PRINTF;
    keywords["scanf"] = Token::TOKEN_SCANF;
}

std::string Token::typeToString() const {
    switch(type) {
        case TOKEN_INT: return "KEYWORD_INT";
        case TOKEN_FLOAT: return "KEYWORD_FLOAT";
        case TOKEN_CHAR: return "KEYWORD_CHAR";
        case TOKEN_VOID: return "KEYWORD_VOID";
        case TOKEN_IF: return "KEYWORD_IF";
        case TOKEN_ELSE: return "KEYWORD_ELSE";
        case TOKEN_WHILE: return "KEYWORD_WHILE";
        case TOKEN_FOR: return "KEYWORD_FOR";
        case TOKEN_RETURN: return "KEYWORD_RETURN";
        case TOKEN_BREAK: return "KEYWORD_BREAK";
        case TOKEN_CONTINUE: return "KEYWORD_CONTINUE";
        case TOKEN_SWITCH: return "KEYWORD_SWITCH";
        case TOKEN_CASE: return "KEYWORD_CASE";
        case TOKEN_DEFAULT: return "KEYWORD_DEFAULT";
        case TOKEN_INCLUDE: return "KEYWORD_INCLUDE";
        case TOKEN_PRINTF: return "FUNCTION_PRINTF";
        case TOKEN_SCANF: return "FUNCTION_SCANF";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_PLUS: return "OPERATOR_PLUS";
        case TOKEN_MINUS: return "OPERATOR_MINUS";
        case TOKEN_MULTIPLY: return "OPERATOR_MULTIPLY";
        case TOKEN_DIVIDE: return "OPERATOR_DIVIDE";
        case TOKEN_MODULO: return "OPERATOR_MODULO";
        case TOKEN_ASSIGN: return "OPERATOR_ASSIGN";
        case TOKEN_EQUALS: return "OPERATOR_EQUALS";
        case TOKEN_NOT_EQUALS: return "OPERATOR_NOT_EQUALS";
        case TOKEN_LESS: return "OPERATOR_LESS";
        case TOKEN_GREATER: return "OPERATOR_GREATER";
        case TOKEN_LESS_EQUAL: return "OPERATOR_LESS_EQUAL";
        case TOKEN_GREATER_EQUAL: return "OPERATOR_GREATER_EQUAL";
        case TOKEN_AND: return "OPERATOR_AND";
        case TOKEN_OR: return "OPERATOR_OR";
        case TOKEN_NOT: return "OPERATOR_NOT";
        case TOKEN_AMPERSAND: return "OPERATOR_AMPERSAND";
        case TOKEN_INCREMENT: return "OPERATOR_INCREMENT";
        case TOKEN_DECREMENT: return "OPERATOR_DECREMENT";
        case TOKEN_SEMICOLON: return "SEPARATOR_SEMICOLON";
        case TOKEN_COMMA: return "SEPARATOR_COMMA";
        case TOKEN_LPAREN: return "SEPARATOR_LPAREN";
        case TOKEN_RPAREN: return "SEPARATOR_RPAREN";
        case TOKEN_LBRACE: return "SEPARATOR_LBRACE";
        case TOKEN_RBRACE: return "SEPARATOR_RBRACE";
        case TOKEN_LBRACKET: return "SEPARATOR_LBRACKET";
        case TOKEN_RBRACKET: return "SEPARATOR_RBRACKET";
        case TOKEN_COLON: return "SEPARATOR_COLON";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

char Lexer::peek() const {
    if (position >= source.length()) return '\0';
    return source[position];
}

char Lexer::peekNext() const {
    if (position + 1 >= source.length()) return '\0';
    return source[position + 1];
}

char Lexer::getChar() {
    if (position >= source.length()) return '\0';
    char c = source[position++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

void Lexer::skipWhitespace() {
    while (isspace(peek())) {
        getChar();
    }
}

bool Lexer::skipComment() {
    if (peek() == '/') {
        char next = peekNext();
        if (next == '/' || next == '*') {
            getChar(); // consume '/'
            if (peek() == '/') {
                // Single-line comment
                while (peek() != '\n' && peek() != '\0') {
                    getChar();
                }
                return true;
            } else if (peek() == '*') {
                // Multi-line comment
                getChar(); // consume '*'
                while (true) {
                    char c = getChar();
                    if (c == '*' && peek() == '/') {
                        getChar(); // consume '/'
                        break;
                    }
                    if (c == '\0') break;
                }
                return true;
            }
        }
    }
    return false;
}

Token Lexer::readIdentifier() {
    std::string value;
    int startLine = line;
    int startCol = column;
    
    while (isalnum(peek()) || peek() == '_') {
        value += getChar();
    }
    
    // Check if it's a keyword
    auto it = keywords.find(value);
    if (it != keywords.end()) {
        return Token(it->second, value, startLine, startCol);
    }
    
    return Token(Token::TOKEN_IDENTIFIER, value, startLine, startCol);
}

Token Lexer::readNumber() {
    std::string value;
    int startLine = line;
    int startCol = column;
    bool isFloat = false;
    
    while (isdigit(peek()) || (peek() == '.' && !isFloat)) {
        if (peek() == '.') isFloat = true;
        value += getChar();
    }
    
    return Token(Token::TOKEN_NUMBER, value, startLine, startCol);
}

Token Lexer::readString() {
    std::string value;
    int startLine = line;
    int startCol = column;
    
    getChar(); // Skip opening quote
    
    while (peek() != '"' && peek() != '\0') {
        if (peek() == '\\') {
            value += getChar(); // Add backslash
        }
        value += getChar();
    }
    
    if (peek() == '"') {
        getChar(); // Skip closing quote
    }
    
    return Token(Token::TOKEN_STRING, value, startLine, startCol);
}

void Lexer::addToken(Token::Type type, const std::string& value) {
    tokens.emplace_back(type, value, line, column);
}

std::vector<Token> Lexer::tokenize() {
    tokens.clear();
    position = 0;
    line = 1;
    column = 1;
    
    while (peek() != '\0') {
        skipWhitespace();
        
        // Skip comments
        if (peek() == '/') {
            if (skipComment()) continue;
        }
        
        // Preprocessor directives (skip #include, #define, etc.)
        if (peek() == '#') {
            while (peek() != '\n' && peek() != '\0') {
                getChar();
            }
            if (peek() == '\n') getChar(); // Skip the newline too
            continue;
        }

        char c = peek();
        if (c == '\0') break;
        
        // Identifiers and keywords
        if (isalpha(c) || c == '_') {
            tokens.push_back(readIdentifier());
            continue;
        }
        
        // Numbers
        if (isdigit(c)) {
            tokens.push_back(readNumber());
            continue;
        }
        
        // Strings
        if (c == '"') {
            tokens.push_back(readString());
            continue;
        }
        
        // Operators and separators
        switch (c) {
            case '+': 
                getChar();
                if (peek() == '+') {
                    addToken(Token::TOKEN_INCREMENT, "++");
                    getChar();
                } else {
                    addToken(Token::TOKEN_PLUS, "+");
                }
                break;
            case '-': 
                getChar();
                if (peek() == '-') {
                    addToken(Token::TOKEN_DECREMENT, "--");
                    getChar();
                } else {
                    addToken(Token::TOKEN_MINUS, "-");
                }
                break;
            case '*': addToken(Token::TOKEN_MULTIPLY, "*"); getChar(); break;
            case '/': addToken(Token::TOKEN_DIVIDE, "/"); getChar(); break;
            case '%': addToken(Token::TOKEN_MODULO, "%"); getChar(); break;
            case '=':
                getChar();
                if (peek() == '=') {
                    addToken(Token::TOKEN_EQUALS, "==");
                    getChar();
                } else {
                    addToken(Token::TOKEN_ASSIGN, "=");
                }
                break;
            case '!':
                getChar();
                if (peek() == '=') {
                    addToken(Token::TOKEN_NOT_EQUALS, "!=");
                    getChar();
                } else {
                    addToken(Token::TOKEN_NOT, "!");
                }
                break;
            case '<':
                getChar();
                if (peek() == '=') {
                    addToken(Token::TOKEN_LESS_EQUAL, "<=");
                    getChar();
                } else {
                    addToken(Token::TOKEN_LESS, "<");
                }
                break;
            case '>':
                getChar();
                if (peek() == '=') {
                    addToken(Token::TOKEN_GREATER_EQUAL, ">=");
                    getChar();
                } else {
                    addToken(Token::TOKEN_GREATER, ">");
                }
                break;
            case '&':
                getChar();
                if (peek() == '&') {
                    addToken(Token::TOKEN_AND, "&&");
                    getChar();
                } else {
                    addToken(Token::TOKEN_AMPERSAND, "&");
                }
                break;
            case '|':
                getChar();
                if (peek() == '|') {
                    addToken(Token::TOKEN_OR, "||");
                    getChar();
                }
                break;
            case ';': addToken(Token::TOKEN_SEMICOLON, ";"); getChar(); break;
            case ':': addToken(Token::TOKEN_COLON, ":"); getChar(); break;
            case ',': addToken(Token::TOKEN_COMMA, ","); getChar(); break;
            case '(': addToken(Token::TOKEN_LPAREN, "("); getChar(); break;
            case ')': addToken(Token::TOKEN_RPAREN, ")"); getChar(); break;
            case '{': addToken(Token::TOKEN_LBRACE, "{"); getChar(); break;
            case '}': addToken(Token::TOKEN_RBRACE, "}"); getChar(); break;
            case '[': addToken(Token::TOKEN_LBRACKET, "["); getChar(); break;
            case ']': addToken(Token::TOKEN_RBRACKET, "]"); getChar(); break;
            default:
                addToken(Token::TOKEN_ERROR, std::string(1, c));
                getChar();
                break;
        }
    }
    
    addToken(Token::TOKEN_EOF);
    return tokens;
}