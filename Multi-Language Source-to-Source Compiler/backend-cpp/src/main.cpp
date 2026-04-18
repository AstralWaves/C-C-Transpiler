#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>

#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/semantic_analyzer.h"
#include "../include/intermediate_code.h"
#include "../include/optimizer.h"
#include "../include/code_generator.h"

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot write to file: " + filename);
    }
    file << content;
}

std::string escapeJson(const std::string& value) {
    std::string escaped;
    for (char ch : value) {
        switch (ch) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (static_cast<unsigned char>(ch) < 0x20) {
                    char buffer[8];
                    std::sprintf(buffer, "\\u%04x", ch);
                    escaped += buffer;
                } else {
                    escaped += ch;
                }
                break;
        }
    }
    return escaped;
}

std::string quoted(const std::string& value) {
    return std::string("\"") + escapeJson(value) + "\"";
}

std::string tokensToJson(const std::vector<Token>& tokens) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < tokens.size(); i++) {
        const auto& token = tokens[i];
        ss << "{\"type\":" << quoted(token.typeToString()) << ",";
        ss << "\"value\":" << quoted(token.value) << ",";
        ss << "\"line\":" << token.line << ",";
        ss << "\"column\":" << token.column << "}";
        if (i + 1 < tokens.size()) ss << ",";
    }
    ss << "]";
    return ss.str();
}

void astToJsonStream(ASTNode* node, std::ostream& ss) {
    if (!node) {
        ss << "null";
        return;
    }
    
    std::string typeName;
    switch (node->type) {
        case NODE_PROGRAM: typeName = "Program"; break;
        case NODE_FUNCTION: typeName = "Function"; break;
        case NODE_VARIABLE_DECL: typeName = "VariableDecl"; break;
        case NODE_ASSIGNMENT: typeName = "Assignment"; break;
        case NODE_BINARY_OP: typeName = "BinaryOp"; break;
        case NODE_UNARY_OP: typeName = "UnaryOp"; break;
        case NODE_IF_STATEMENT: typeName = "IfStatement"; break;
        case NODE_WHILE_LOOP: typeName = "WhileLoop"; break;
        case NODE_FOR_LOOP: typeName = "ForLoop"; break;
        case NODE_SWITCH_STATEMENT: typeName = "SwitchStatement"; break;
        case NODE_CASE_STATEMENT: typeName = "CaseStatement"; break;
        case NODE_BREAK_STATEMENT: typeName = "BreakStatement"; break;
        case NODE_CONTINUE_STATEMENT: typeName = "ContinueStatement"; break;
        case NODE_RETURN_STATEMENT: typeName = "ReturnStatement"; break;
        case NODE_CALL_EXPRESSION: typeName = "CallExpression"; break;
        case NODE_NUMBER_LITERAL: typeName = "NumberLiteral"; break;
        case NODE_STRING_LITERAL: typeName = "StringLiteral"; break;
        case NODE_IDENTIFIER: typeName = "Identifier"; break;
        case NODE_PARAMETER: typeName = "Parameter"; break;
        case NODE_BLOCK: typeName = "Block"; break;
        default: typeName = "Unknown";
    }
    
    ss << "{\"type\":\"" << typeName << "\",";
    ss << "\"value\":" << quoted(node->value) << ",";
    ss << "\"line\":" << node->line << ",";
    ss << "\"column\":" << node->column << ",";
    ss << "\"children\": [";
    for (size_t i = 0; i < node->children.size(); i++) {
        astToJsonStream(node->children[i].get(), ss);
        if (i + 1 < node->children.size()) ss << ",";
    }
    ss << "]}";
}

std::string astToJson(ASTNode* node) {
    std::stringstream ss;
    astToJsonStream(node, ss);
    return ss.str();
}

std::string intermediateToJson(const std::vector<Quadruple>& code) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < code.size(); i++) {
        const auto& quad = code[i];
        ss << "{\"op\":" << quoted(quad.opToString()) << ",";
        ss << "\"arg1\":" << quoted(quad.arg1) << ",";
        ss << "\"arg2\":" << quoted(quad.arg2) << ",";
        ss << "\"result\":" << quoted(quad.result) << "}";
        if (i + 1 < code.size()) ss << ",";
    }
    ss << "]";
    return ss.str();
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0] << " <input_file> <target_language> <output_file>" << std::endl;
            return 1;
        }
        
        std::string inputFile = argv[1];
        std::string targetLanguage = argv[2];
        std::string outputFile = argv[3];
        
        // Validate target language
        if (targetLanguage != "python" && targetLanguage != "java" && 
            targetLanguage != "javascript" && targetLanguage != "cpp") {
            throw std::runtime_error("Unsupported target language: " + targetLanguage);
        }
        
        // Phase 1: Read source code
        std::string sourceCode = readFile(inputFile);
        
        // Phase 2: Lexical Analysis
        Lexer lexer(sourceCode);
        std::vector<Token> tokens = lexer.tokenize();
        
        // Phase 3: Syntax Analysis (Parsing)
        Parser parser(tokens);
        ASTNode* ast = parser.parse();
        
        if (!ast) {
            throw std::runtime_error("Parsing failed");
        }
        
        // Phase 4: Semantic Analysis
        SemanticAnalyzer semanticAnalyzer;
        semanticAnalyzer.analyze(ast);
        
        // Phase 5: Intermediate Code Generation
        IntermediateCodeGenerator icg;
        std::vector<Quadruple> intermediateCode = icg.generate(ast);
        
        // Phase 6: Code Optimization
        Optimizer optimizer;
        std::vector<Quadruple> optimizedCode = optimizer.optimize(intermediateCode);
        
        // Phase 7: Target Code Generation
        CodeGenerator codeGenerator(semanticAnalyzer.getSymbolTable(), targetLanguage);
        std::string targetCode = codeGenerator.generate(optimizedCode);
        
        // Write output
        writeFile(outputFile, targetCode);
        
        // Output JSON for frontend
        std::cout << "---COMPILER_OUTPUT_START---\n";
        std::cout << "{";
        std::cout << "\"success\":true,";
        std::cout << "\"tokens\":" << tokensToJson(tokens) << ",";
        std::cout << "\"ast\":" << astToJson(ast) << ",";
        std::cout << "\"intermediate_code\":" << intermediateToJson(intermediateCode) << ",";
        std::cout << "\"optimized_code\":" << intermediateToJson(optimizedCode) << ",";
        std::cout << "\"target_code\":" << quoted(targetCode);
        std::cout << "}" << std::endl;
        std::cout << "---COMPILER_OUTPUT_END---\n";
        
        delete ast;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "---COMPILER_OUTPUT_START---\n";
        std::cout << "{\"success\":false,\"error\":" << quoted(e.what()) << "}" << std::endl;
        std::cout << "---COMPILER_OUTPUT_END---\n";
        return 0;
    }
}