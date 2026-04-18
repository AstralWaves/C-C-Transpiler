#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>

struct Token {
    enum Type {
        // Keywords
        TOKEN_INT, TOKEN_FLOAT, TOKEN_CHAR, TOKEN_VOID,
        TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE, TOKEN_FOR,
        TOKEN_RETURN, TOKEN_BREAK, TOKEN_CONTINUE,
        TOKEN_SWITCH, TOKEN_CASE, TOKEN_DEFAULT,
        TOKEN_INCLUDE, TOKEN_PRINTF, TOKEN_SCANF,
        
        // Identifiers and literals
        TOKEN_IDENTIFIER, TOKEN_NUMBER, TOKEN_STRING,
        
        // Operators
        TOKEN_PLUS, TOKEN_MINUS, TOKEN_MULTIPLY, TOKEN_DIVIDE, TOKEN_MODULO,
        TOKEN_ASSIGN, TOKEN_EQUALS, TOKEN_NOT_EQUALS,
        TOKEN_LESS, TOKEN_GREATER, TOKEN_LESS_EQUAL, TOKEN_GREATER_EQUAL,
        TOKEN_AND, TOKEN_OR, TOKEN_NOT, TOKEN_AMPERSAND,
        
        // Increment/Decrement
        TOKEN_INCREMENT, TOKEN_DECREMENT,
        
        // Separators
        TOKEN_SEMICOLON, TOKEN_COMMA, TOKEN_LPAREN, TOKEN_RPAREN,
        TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_LBRACKET, TOKEN_RBRACKET,
        TOKEN_COLON,
        
        // Special
        TOKEN_EOF, TOKEN_ERROR
    };
    
    Type type;
    std::string value;
    int line;
    int column;
    
    Token(Type t, const std::string& v = "", int l = 1, int c = 1)
        : type(t), value(v), line(l), column(c) {}
    
    std::string typeToString() const;
};

class Lexer {
private:
    std::string source;
    size_t position;
    int line;
    int column;
    std::vector<Token> tokens;
    
    // Keyword mapping
    std::unordered_map<std::string, Token::Type> keywords;
    
    char peek() const;
    char peekNext() const;
    char getChar();
    void skipWhitespace();
    bool skipComment();
    Token readIdentifier();
    Token readNumber();
    Token readString();
    void addToken(Token::Type type, const std::string& value = "");
    
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();
    const std::vector<Token>& getTokens() const { return tokens; }
};

#endif