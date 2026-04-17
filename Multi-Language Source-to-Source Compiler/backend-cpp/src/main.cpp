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
    std::string json = "[";
    for (size_t i = 0; i < tokens.size(); i++) {
        const auto& token = tokens[i];
        json += "{";
        json += "\"type\":" + quoted(token.typeToString()) + ",";
        json += "\"value\":" + quoted(token.value) + ",";
        json += "\"line\":" + std::to_string(token.line) + ",";
        json += "\"column\":" + std::to_string(token.column);
        json += "}";
        if (i + 1 < tokens.size()) json += ",";
    }
    json += "]";
    return json;
}

std::string astToJson(ASTNode* node) {
    if (!node) return "null";
    std::string json = "{";
    json += "\"type\":\"" + std::to_string(node->type) + "\",";
    json += "\"value\":" + quoted(node->value) + ",";
    json += "\"children\": [";
    for (size_t i = 0; i < node->children.size(); i++) {
        json += astToJson(node->children[i].get());
        if (i + 1 < node->children.size()) json += ",";
    }
    json += "]";
    json += "}";
    return json;
}

std::string intermediateToJson(const std::vector<Quadruple>& code) {
    std::string json = "[";
    for (size_t i = 0; i < code.size(); i++) {
        const auto& quad = code[i];
        json += "{";
        json += "\"op\":" + quoted(quad.opToString()) + ",";
        json += "\"arg1\":" + quoted(quad.arg1) + ",";
        json += "\"arg2\":" + quoted(quad.arg2) + ",";
        json += "\"result\":" + quoted(quad.result);
        json += "}";
        if (i + 1 < code.size()) json += ",";
    }
    json += "]";
    return json;
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
        std::cerr << "---COMPILER_OUTPUT_START---\n";
        std::cerr << "{\"success\":false,\"error\":" << quoted(e.what()) << "}" << std::endl;
        std::cerr << "---COMPILER_OUTPUT_END---\n";
        return 1;
    }
}