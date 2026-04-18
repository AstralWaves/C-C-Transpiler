// PHASE 6: TARGET CODE GENERATION (SOURCE-TO-SOURCE)
#include "../include/code_generator.h"
#include <sstream>
#include <algorithm>
#include <regex>
#include <cctype>
#include <set>

CodeGenerator::CodeGenerator(SymbolTable& symTable, const std::string& target)
    : symbolTable(symTable), targetLanguage(target) {}

std::string CodeGenerator::generate(const std::vector<Quadruple>& intermediateCode) {
    if (targetLanguage == "python") {
        return generatePython(intermediateCode);
    } else if (targetLanguage == "java") {
        return generateJava(intermediateCode);
    } else if (targetLanguage == "javascript") {
        return generateJavaScript(intermediateCode);
    } else if (targetLanguage == "cpp") {
        return generateCpp(intermediateCode);
    }
    return "";
}

std::string CodeGenerator::generatePython(const std::vector<Quadruple>& code) {
    std::ostringstream output;
    int indentLevel = 0;
    std::set<std::string> declaredVars;
    
    output << "# Generated Python code\n";
    output << "# Converted from C/C++ by C-Transpiler\n\n";
    
    for (const auto& quad : code) {
        std::string line;
        
        switch (quad.op) {
            case Quadruple::OP_ASSIGN:
                // Don't redeclare variables in Python
                line = quad.result + " = " + convertExpression(quad.arg1);
                declaredVars.insert(quad.result);
                break;
                
            case Quadruple::OP_ADD:
            case Quadruple::OP_SUB:
            case Quadruple::OP_MUL:
            case Quadruple::OP_DIV:
            case Quadruple::OP_MOD:
            case Quadruple::OP_LT:
            case Quadruple::OP_GT:
            case Quadruple::OP_LE:
            case Quadruple::OP_GE:
            case Quadruple::OP_EQ:
            case Quadruple::OP_NE:
            case Quadruple::OP_AND:
            case Quadruple::OP_OR:
            case Quadruple::OP_NOT:
                line = emitOperatorQuad(quad);
                declaredVars.insert(quad.result);
                break;
                
            case Quadruple::OP_IF:
                line = "if " + convertExpression(quad.arg1) + ":";
                break;
                
            case Quadruple::OP_ELSE:
                if (indentLevel > 0) indentLevel--;
                line = "else:";
                break;
                
            case Quadruple::OP_BLOCK_END:
                if (indentLevel > 0) indentLevel--;
                line = "";
                break;
                
            case Quadruple::OP_WHILE:
                if (quad.arg1 == "1") {
                    line = "while True:";
                } else {
                    line = "while " + convertExpression(quad.arg1) + ":";
                }
                break;
                
            case Quadruple::OP_JMP_FALSE:
                line = "if not (" + convertExpression(quad.arg1) + "):\n" + indent(indentLevel + 1) + "break";
                break;
                
            case Quadruple::OP_JMP:
            case Quadruple::OP_LABEL:
                continue;
                
            case Quadruple::OP_PRINTF:
                if (quad.arg2.empty()) {
                    line = "print(" + formatPrintfString(quad.arg1) + ")";
                } else {
                    line = "print(" + formatPrintfString(quad.arg1) + " % (" + quad.arg2 + "))";
                }
                break;
                
            case Quadruple::OP_SCANF:
                line = quad.arg1 + " = int(input())";
                declaredVars.insert(quad.arg1);
                break;
                
            case Quadruple::OP_RETURN:
                if (quad.arg1.empty()) {
                    line = "return";
                } else {
                    line = "return " + convertExpression(quad.arg1);
                }
                break;
                
            case Quadruple::OP_FUNC_START:
                if (!quad.arg1.empty()) {
                    output << "\n" << indent(indentLevel) << "def " << quad.result << "(" << quad.arg1 << "):\n";
                } else {
                    output << "\n" << indent(indentLevel) << "def " << quad.result << "():\n";
                }
                indentLevel++;
                continue;
                
            case Quadruple::OP_FUNC_END:
                if (indentLevel > 0) {
                    indentLevel--;
                }
                output << indent(indentLevel) << "\n";
                continue;
                
            case Quadruple::OP_CALL:
                if (!quad.arg2.empty()) {
                    line = quad.result + " = " + quad.arg1 + "(" + quad.arg2 + ")";
                } else {
                    line = quad.result + " = " + quad.arg1 + "()";
                }
                declaredVars.insert(quad.result);
                break;
                
            default:
                continue;
        }
        
        if (!line.empty()) {
            output << indent(indentLevel) << line << "\n";
        }
        if (quad.op == Quadruple::OP_IF || quad.op == Quadruple::OP_WHILE || quad.op == Quadruple::OP_ELSE) {
            indentLevel++;
        }
    }
    
    return output.str();
}

std::string CodeGenerator::generateJava(const std::vector<Quadruple>& code) {
    std::ostringstream output;
    int indentLevel = 1;
    bool inMain = false;
    bool hasMain = false;
    bool scannerDeclared = false;
    std::set<std::string> declaredVars;
    
    output << "// Generated Java code\n";
    output << "// Converted from C/C++ by C-Transpiler\n\n";
    output << "public class GeneratedCode {\n";
    
    for (const auto& quad : code) {
        std::string line;
        
        switch (quad.op) {
            case Quadruple::OP_ASSIGN:
                if (declaredVars.find(quad.result) == declaredVars.end()) {
                    line = "int " + quad.result + " = " + convertExpression(quad.arg1) + ";";
                    declaredVars.insert(quad.result);
                } else {
                    line = quad.result + " = " + convertExpression(quad.arg1) + ";";
                }
                break;
                
            case Quadruple::OP_ADD:
            case Quadruple::OP_SUB:
            case Quadruple::OP_MUL:
            case Quadruple::OP_DIV:
            case Quadruple::OP_MOD:
            case Quadruple::OP_LT:
            case Quadruple::OP_GT:
            case Quadruple::OP_LE:
            case Quadruple::OP_GE:
            case Quadruple::OP_EQ:
            case Quadruple::OP_NE:
            case Quadruple::OP_AND:
            case Quadruple::OP_OR:
            case Quadruple::OP_NOT: {
                line = emitOperatorQuad(quad);
                declaredVars.insert(quad.result);
                break;
            }
                
            case Quadruple::OP_PRINTF:
                if (quad.arg2.empty()) {
                    line = "System.out.println(" + formatPrintfString(quad.arg1) + ");";
                } else {
                    // Convert format string for Java string concatenation
                    std::string formatStr = convertPrintfToJava(quad.arg1);
                    // Replace {} placeholders with the actual argument
                    size_t placeholderPos = formatStr.find("{}");
                    if (placeholderPos != std::string::npos) {
                        formatStr.replace(placeholderPos, 2, "\" + " + quad.arg2 + " + \"");
                    }
                    line = "System.out.println(\"" + formatStr + "\");";
                }
                break;
                
            case Quadruple::OP_SCANF:
                if (!scannerDeclared && inMain) {
                    output << indent(indentLevel) << "java.util.Scanner scanner = new java.util.Scanner(System.in);\n";
                    scannerDeclared = true;
                }
                if (declaredVars.find(quad.arg1) == declaredVars.end()) {
                    line = "int " + quad.arg1 + " = scanner.nextInt();";
                    declaredVars.insert(quad.arg1);
                } else {
                    line = quad.arg1 + " = scanner.nextInt();";
                }
                break;
                
            case Quadruple::OP_RETURN:
                if (inMain) {
                    line = "return;";
                } else if (quad.arg1.empty()) {
                    line = "return;";
                } else {
                    line = "return " + convertExpression(quad.arg1) + ";";
                }
                break;
                
            case Quadruple::OP_IF:
                line = "if " + javaIntCondition(quad.arg1) + " {";
                indentLevel++;
                break;
                
            case Quadruple::OP_WHILE:
                if (quad.arg1 == "1") {
                    line = "while (true) {";
                } else {
                    line = "while " + javaIntCondition(quad.arg1) + " {";
                }
                indentLevel++;
                break;
                
            case Quadruple::OP_BLOCK_END:
                if (indentLevel > 0) indentLevel--;
                line = "}";
                break;
                
            case Quadruple::OP_JMP_FALSE:
                line = "if (" + convertExpression(quad.arg1) + " == 0) break;";
                break;
                
            case Quadruple::OP_ELSE_IF:
                indentLevel--;
                line = "} else if " + javaIntCondition(quad.arg1) + " {";
                indentLevel++;
                break;
                
            case Quadruple::OP_ELSE:
                indentLevel--;
                line = "} else {";
                indentLevel++;
                break;
                
            case Quadruple::OP_LABEL:
                line = quad.result + ":";
                output << indent(indentLevel) << line << "\n";
                continue;
                
            case Quadruple::OP_JMP:
                line = "// goto " + quad.result + ";";
                break;
                
            case Quadruple::OP_FUNC_START:
                if (quad.result == "main") {
                    line = "public static void main(String[] args) {";
                    inMain = true;
                    hasMain = true;
                } else {
                    if (!quad.arg1.empty()) {
                        line = "public static int " + quad.result + "(" + quad.arg1 + ") {";
                    } else {
                        line = "public static int " + quad.result + "() {";
                    }
                }
                indentLevel++;
                break;
                
            case Quadruple::OP_FUNC_END:
                indentLevel--;
                line = "}";
                inMain = false;
                break;
                
            case Quadruple::OP_CALL:
                if (declaredVars.find(quad.result) == declaredVars.end()) {
                    if (!quad.arg2.empty()) {
                        line = "int " + quad.result + " = " + quad.arg1 + "(" + quad.arg2 + ");";
                    } else {
                        line = "int " + quad.result + " = " + quad.arg1 + "();";
                    }
                    declaredVars.insert(quad.result);
                } else {
                    if (!quad.arg2.empty()) {
                        line = quad.result + " = " + quad.arg1 + "(" + quad.arg2 + ");";
                    } else {
                        line = quad.result + " = " + quad.arg1 + "();";
                    }
                }
                break;
                
            case Quadruple::OP_EXIT:
                line = "System.exit(" + convertExpression(quad.arg1) + ");";
                break;
                
            default:
                continue;
        }
        
        if (!line.empty()) {
            output << indent(indentLevel) << line << "\n";
        }
    }
    
    if (!hasMain) {
        output << "\n" << indent(1) << "public static void main(String[] args) {\n";
        output << indent(2) << "System.out.println(\"No main function found\");\n";
        output << indent(1) << "}\n";
    }
    
    output << "}\n";
    return output.str();
}

static std::string unescapeCLiteralBody(const std::string& in) {
    std::string out;
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '\\' && i + 1 < in.size()) {
            char n = in[i + 1];
            if (n == 'n') {
                out += '\n';
                i++;
                continue;
            }
            if (n == 't') {
                out += '\t';
                i++;
                continue;
            }
            if (n == 'r') {
                out += '\r';
                i++;
                continue;
            }
            if (n == '\\') {
                out += '\\';
                i++;
                continue;
            }
            if (n == '"') {
                out += '"';
                i++;
                continue;
            }
        }
        out += in[i];
    }
    return out;
}

std::string CodeGenerator::formatPrintfStringUtilJs(const std::string& format) {
    if (format.empty()) return "\"\"";
    std::string result = format;
    if (!result.empty() && result.front() == '"') {
        result = result.substr(1);
    }
    if (!result.empty() && result.back() == '"') {
        result = result.substr(0, result.length() - 1);
    }
    result = unescapeCLiteralBody(result);
    size_t pos = 0;
    while ((pos = result.find("%%", pos)) != std::string::npos) {
        result.replace(pos, 2, "%");
        pos++;
    }
    std::string escaped;
    for (size_t i = 0; i < result.size(); ++i) {
        char c = result[i];
        if (c == '\\') escaped += "\\\\";
        else if (c == '"') escaped += "\\\"";
        else if (c == '\n') escaped += "\\n";
        else if (c == '\r') escaped += "\\r";
        else if (c == '\t') escaped += "\\t";
        else escaped += c;
    }
    return "\"" + escaped + "\"";
}

std::string CodeGenerator::jsEmitPrintf(const Quadruple& quad) {
    if (quad.arg2.empty()) {
        return "console.log(" + formatPrintfString(quad.arg1) + ");";
    }
    return "console.log(util.format(" + formatPrintfStringUtilJs(quad.arg1) + ", " + quad.arg2 + "));";
}

std::string CodeGenerator::generateJavaScript(const std::vector<Quadruple>& code) {
    std::ostringstream output;
    int indentLevel = 0;
    std::set<std::string> declaredVars;
    bool hasScanf = false;
    for (const auto& q : code) {
        if (q.op == Quadruple::OP_SCANF) {
            hasScanf = true;
            break;
        }
    }
    
    output << "'use strict';\n\n";
    output << "// Transpiled from C-like source (subset). Target: Node.js\n";
    output << "// Run: node <this-file.js>\n";
    output << "// For scanf: pipe input, e.g. echo \"42\" | node <this-file.js>\n\n";
    output << "const util = require('util');\n";
    if (hasScanf) {
        output << "const fs = require('fs');\n";
        output << "const _stdinTokens = fs.readFileSync(0, 'utf8').trim().split(/\\s+/);\n";
        output << "let _stdinIdx = 0;\n\n";
    } else {
        output << "\n";
    }
    
    for (const auto& quad : code) {
        std::string line;
        
        switch (quad.op) {
            case Quadruple::OP_ASSIGN:
                if (declaredVars.find(quad.result) == declaredVars.end()) {
                    line = "let " + quad.result + " = " + convertExpression(quad.arg1) + ";";
                    declaredVars.insert(quad.result);
                } else {
                    line = quad.result + " = " + convertExpression(quad.arg1) + ";";
                }
                break;
                
            case Quadruple::OP_ADD:
            case Quadruple::OP_SUB:
            case Quadruple::OP_MUL:
            case Quadruple::OP_DIV:
            case Quadruple::OP_MOD:
            case Quadruple::OP_LT:
            case Quadruple::OP_GT:
            case Quadruple::OP_LE:
            case Quadruple::OP_GE:
            case Quadruple::OP_EQ:
            case Quadruple::OP_NE:
            case Quadruple::OP_AND:
            case Quadruple::OP_OR:
            case Quadruple::OP_NOT:
                line = emitOperatorQuad(quad);
                declaredVars.insert(quad.result);
                break;
                
            case Quadruple::OP_PRINTF:
                line = jsEmitPrintf(quad);
                break;
                
            case Quadruple::OP_SCANF: {
                std::string rhs = hasScanf
                    ? std::string("parseInt(_stdinTokens[_stdinIdx++], 10)")
                    : std::string("parseInt(prompt(), 10)");
                if (declaredVars.find(quad.arg1) == declaredVars.end()) {
                    line = "let " + quad.arg1 + " = " + rhs + ";";
                    declaredVars.insert(quad.arg1);
                } else {
                    line = quad.arg1 + " = " + rhs + ";";
                }
                break;
            }
                
            case Quadruple::OP_RETURN:
                if (quad.arg1.empty()) {
                    line = "return;";
                } else {
                    line = "return " + convertExpression(quad.arg1) + ";";
                }
                break;
                
            case Quadruple::OP_IF:
                line = "if (" + convertExpression(quad.arg1) + ") {";
                break;
                
            case Quadruple::OP_ELSE:
                if (indentLevel > 0) indentLevel--;
                line = "} else {";
                break;
                
            case Quadruple::OP_ELSE_IF:
                if (indentLevel > 0) indentLevel--;
                line = "} else if (" + convertExpression(quad.arg1) + ") {";
                break;
                
            case Quadruple::OP_BLOCK_END:
                if (indentLevel > 0) indentLevel--;
                line = "}";
                break;
                
            case Quadruple::OP_WHILE:
                if (quad.arg1 == "1") {
                    line = "while (true) {";
                } else {
                    line = "while (" + convertExpression(quad.arg1) + ") {";
                }
                break;
                
            case Quadruple::OP_JMP_FALSE:
                line = "if (!(" + convertExpression(quad.arg1) + ")) break;";
                break;
                
            case Quadruple::OP_JMP:
                break;
                
            case Quadruple::OP_LABEL:
                continue;
                
            case Quadruple::OP_FUNC_START:
                if (!quad.arg1.empty()) {
                    line = "function " + quad.result + "(" + quad.arg1 + ") {";
                } else {
                    line = "function " + quad.result + "() {";
                }
                break;
                
            case Quadruple::OP_FUNC_END:
                if (indentLevel > 0) indentLevel--;
                line = "}";
                break;
                
            case Quadruple::OP_CALL:
                if (declaredVars.find(quad.result) == declaredVars.end()) {
                    if (!quad.arg2.empty()) {
                        line = "let " + quad.result + " = " + quad.arg1 + "(" + quad.arg2 + ");";
                    } else {
                        line = "let " + quad.result + " = " + quad.arg1 + "();";
                    }
                    declaredVars.insert(quad.result);
                } else {
                    if (!quad.arg2.empty()) {
                        line = quad.result + " = " + quad.arg1 + "(" + quad.arg2 + ");";
                    } else {
                        line = quad.result + " = " + quad.arg1 + "();";
                    }
                }
                break;
                
            case Quadruple::OP_EXIT:
                line = "process.exit(" + convertExpression(quad.arg1) + ");";
                break;
                
            default:
                continue;
        }
        
        if (!line.empty()) {
            output << indent(indentLevel) << line << "\n";
        }
        if (quad.op == Quadruple::OP_IF || quad.op == Quadruple::OP_WHILE || quad.op == Quadruple::OP_ELSE ||
            quad.op == Quadruple::OP_ELSE_IF) {
            indentLevel++;
        }
        if (quad.op == Quadruple::OP_FUNC_START) {
            indentLevel++;
        }
    }
    
    output << "\n// Run entry point\n";
    output << "main();\n";
    
    return output.str();
}

std::string CodeGenerator::generateCpp(const std::vector<Quadruple>& code) {
    std::ostringstream output;
    int indentLevel = 0;
    std::set<std::string> declaredVars;
    
    output << "// Generated C++ code\n";
    output << "#include <iostream>\n";
    output << "#include <cstdio>\n";
    output << "#include <string>\n";
    output << "using namespace std;\n\n";
    
    for (const auto& quad : code) {
        std::string line;
        
        switch (quad.op) {
            case Quadruple::OP_ASSIGN:
                if (declaredVars.find(quad.result) == declaredVars.end()) {
                    line = "int " + quad.result + " = " + convertExpression(quad.arg1) + ";";
                    declaredVars.insert(quad.result);
                } else {
                    line = quad.result + " = " + convertExpression(quad.arg1) + ";";
                }
                break;
                
            case Quadruple::OP_ADD:
            case Quadruple::OP_SUB:
            case Quadruple::OP_MUL:
            case Quadruple::OP_DIV:
            case Quadruple::OP_MOD:
            case Quadruple::OP_LT:
            case Quadruple::OP_GT:
            case Quadruple::OP_LE:
            case Quadruple::OP_GE:
            case Quadruple::OP_EQ:
            case Quadruple::OP_NE:
            case Quadruple::OP_AND:
            case Quadruple::OP_OR:
            case Quadruple::OP_NOT:
                line = emitOperatorQuad(quad);
                declaredVars.insert(quad.result);
                break;
                
            case Quadruple::OP_PRINTF:
                if (quad.arg2.empty()) {
                    line = "cout << " + formatPrintfString(quad.arg1) + " << endl;";
                } else {
                    line = "printf(" + convertExpression(quad.arg1) + ", " + quad.arg2 + ");";
                }
                break;
                
            case Quadruple::OP_SCANF:
                line = "cin >> " + quad.arg1 + ";";
                declaredVars.insert(quad.arg1);
                break;
                
            case Quadruple::OP_RETURN:
                if (quad.arg1.empty()) {
                    line = "return;";
                } else {
                    line = "return " + convertExpression(quad.arg1) + ";";
                }
                break;
                
            case Quadruple::OP_IF:
                line = "if (" + convertExpression(quad.arg1) + ") {";
                indentLevel++;
                break;
                
            case Quadruple::OP_ELSE:
                indentLevel--;
                line = "} else {";
                indentLevel++;
                break;
                
            case Quadruple::OP_ELSE_IF:
                indentLevel--;
                line = "} else if (" + convertExpression(quad.arg1) + ") {";
                indentLevel++;
                break;
                
            case Quadruple::OP_BLOCK_END:
                if (indentLevel > 0) indentLevel--;
                line = "}";
                break;
                
            case Quadruple::OP_WHILE:
                if (quad.arg1 == "1") {
                    line = "while (true) {";
                } else {
                    line = "while (" + convertExpression(quad.arg1) + ") {";
                }
                indentLevel++;
                break;
                
            case Quadruple::OP_JMP_FALSE:
                line = "if (!(" + convertExpression(quad.arg1) + ")) goto " + quad.result + ";";
                break;
                
            case Quadruple::OP_LABEL:
                line = quad.result + ":";
                break;
                
            case Quadruple::OP_JMP:
                line = "goto " + quad.result + ";";
                break;
                
            case Quadruple::OP_FUNC_START:
                if (!quad.arg1.empty()) {
                    line = "int " + quad.result + "(" + quad.arg1 + ") {";
                } else {
                    line = "int " + quad.result + "() {";
                }
                indentLevel++;
                break;
                
            case Quadruple::OP_FUNC_END:
                indentLevel--;
                line = "}";
                break;
                
            case Quadruple::OP_CALL:
                if (declaredVars.find(quad.result) == declaredVars.end()) {
                    if (!quad.arg2.empty()) {
                        line = "int " + quad.result + " = " + quad.arg1 + "(" + quad.arg2 + ");";
                    } else {
                        line = "int " + quad.result + " = " + quad.arg1 + "();";
                    }
                    declaredVars.insert(quad.result);
                } else {
                    if (!quad.arg2.empty()) {
                        line = quad.result + " = " + quad.arg1 + "(" + quad.arg2 + ");";
                    } else {
                        line = quad.result + " = " + quad.arg1 + "();";
                    }
                }
                break;
                
            default:
                continue;
        }
        
        if (!line.empty()) {
            output << indent(indentLevel) << line << "\n";
        }
    }
    
    return output.str();
}

std::string CodeGenerator::convertExpression(const std::string& expr) {
    if (expr.empty()) return "";
    
    std::string result = expr;
    
    // Handle arithmetic expressions
    // Convert C-style operators if needed for the target language
    if (targetLanguage == "python") {
        // No changes needed for basic operators
        result = expr;
    } else if (targetLanguage == "java") {
        result = expr;
    } else if (targetLanguage == "javascript") {
        result = expr;
    }
    
    return result;
}

std::string CodeGenerator::formatPrintfString(const std::string& format) {
    if (format.empty()) return "\"\"";
    
    std::string result = format;
    
    // Remove surrounding quotes if present
    if (!result.empty() && result.front() == '"') {
        result = result.substr(1);
    }
    if (!result.empty() && result.back() == '"') {
        result = result.substr(0, result.length() - 1);
    }
    
    // Handle format specifiers for different target languages
    if (targetLanguage == "python") {
        // Keep C-style %d/%s for Python's "..." % (args) formatting
        size_t pos = 0;
        while ((pos = result.find("%%", pos)) != std::string::npos) {
            result.replace(pos, 2, "%");
            pos++;
        }
    } else if (targetLanguage == "java") {
        // Convert C printf to Java System.out.println
        // %d -> {} (for integer), %s -> {}, %f -> {}, \n -> actual newline
        result = convertPrintfToJava(result);
    } else if (targetLanguage == "javascript") {
        // Convert C printf to JavaScript console.log
        result = convertPrintfToJs(result);
    }
    
    // Add quotes back
    return "\"" + result + "\"";
}

std::string CodeGenerator::convertPrintfToJava(const std::string& format) {
    std::string result = format;
    
    // Replace \n (backslash-n) with actual newline for Java
    size_t pos = 0;
    while ((pos = result.find("\\n", pos)) != std::string::npos) {
        result.replace(pos, 2, "\n");
        pos += 1;
    }
    
    // Replace format specifiers with + for string concatenation
    // %d, %i -> {} (placeholder for integer)
    std::regex intSpec(R"(%[di])");
    result = std::regex_replace(result, intSpec, "{}");
    
    std::regex floatSpec(R"(%[fFe])");
    result = std::regex_replace(result, floatSpec, "{}");
    
    std::regex stringSpec(R"(%s)");
    result = std::regex_replace(result, stringSpec, "{}");
    
    std::regex charSpec(R"(%c)");
    result = std::regex_replace(result, charSpec, "{}");
    
    // Handle %% as single %
    pos = 0;
    while ((pos = result.find("%%", pos)) != std::string::npos) {
        result.replace(pos, 2, "%");
        pos++;
    }
    
    return result;
}

std::string CodeGenerator::convertPrintfToJs(const std::string& format) {
    std::string result = format;
    
    // Replace \n with actual newline
    size_t pos = 0;
    while ((pos = result.find("\\n", pos)) != std::string::npos) {
        result.replace(pos, 2, "\\n");
        pos += 2;
    }
    
    // Replace format specifiers with template literal placeholders
    std::regex intSpec(R"(%[di])");
    result = std::regex_replace(result, intSpec, "{}");
    
    std::regex floatSpec(R"(%[fFe])");
    result = std::regex_replace(result, floatSpec, "{}");
    
    std::regex stringSpec(R"(%s)");
    result = std::regex_replace(result, stringSpec, "{}");
    
    std::regex charSpec(R"(%c)");
    result = std::regex_replace(result, charSpec, "{}");
    
    // Handle %% as single %
    pos = 0;
    while ((pos = result.find("%%", pos)) != std::string::npos) {
        result.replace(pos, 2, "%");
        pos++;
    }
    
    return result;
}

std::string CodeGenerator::javaIntCondition(const std::string& expr) {
    return "(" + convertExpression(expr) + " != 0)";
}

std::string CodeGenerator::emitOperatorQuad(const Quadruple& quad) {
    const std::string& r = quad.result;
    const std::string& a = quad.arg1;
    const std::string& b = quad.arg2;
    const std::string ca = convertExpression(a);
    const std::string cb = convertExpression(b);

    if (quad.op == Quadruple::OP_SUB && a == "0") {
        if (targetLanguage == "python") return r + " = -" + cb;
        if (targetLanguage == "java") return "int " + r + " = -" + cb + ";";
        if (targetLanguage == "javascript") return "let " + r + " = -" + cb + ";";
        return "int " + r + " = -" + cb + ";";
    }
    if (quad.op == Quadruple::OP_NOT) {
        if (targetLanguage == "python") return r + " = not " + ca;
        if (targetLanguage == "java") return "int " + r + " = (!" + ca + ") ? 1 : 0;";
        if (targetLanguage == "javascript") return "let " + r + " = !" + ca + ";";
        return "int " + r + " = !" + ca + ";";
    }

    auto rel = [&](const std::string& op) -> std::string {
        if (targetLanguage == "python") return r + " = int(" + ca + " " + op + " " + cb + ")";
        if (targetLanguage == "java") return "int " + r + " = (" + ca + " " + op + " " + cb + ") ? 1 : 0;";
        if (targetLanguage == "javascript") return "let " + r + " = (" + ca + " " + op + " " + cb + ") ? 1 : 0;";
        return "int " + r + " = " + ca + " " + op + " " + cb + ";";
    };

    switch (quad.op) {
        case Quadruple::OP_ADD:
            if (targetLanguage == "python") return r + " = " + ca + " + " + cb;
            if (targetLanguage == "java") return "int " + r + " = " + ca + " + " + cb + ";";
            if (targetLanguage == "javascript") return "let " + r + " = " + ca + " + " + cb + ";";
            return "int " + r + " = " + ca + " + " + cb + ";";
        case Quadruple::OP_SUB:
            if (targetLanguage == "python") return r + " = " + ca + " - " + cb;
            if (targetLanguage == "java") return "int " + r + " = " + ca + " - " + cb + ";";
            if (targetLanguage == "javascript") return "let " + r + " = " + ca + " - " + cb + ";";
            return "int " + r + " = " + ca + " - " + cb + ";";
        case Quadruple::OP_MUL:
            if (targetLanguage == "python") return r + " = " + ca + " * " + cb;
            if (targetLanguage == "java") return "int " + r + " = " + ca + " * " + cb + ";";
            if (targetLanguage == "javascript") return "let " + r + " = " + ca + " * " + cb + ";";
            return "int " + r + " = " + ca + " * " + cb + ";";
        case Quadruple::OP_DIV:
            if (targetLanguage == "python") return r + " = " + ca + " // " + cb;
            if (targetLanguage == "java") return "int " + r + " = " + ca + " / " + cb + ";";
            if (targetLanguage == "javascript") return "let " + r + " = Math.floor(" + ca + " / " + cb + ");";
            return "int " + r + " = " + ca + " / " + cb + ";";
        case Quadruple::OP_MOD:
            if (targetLanguage == "python") return r + " = " + ca + " % " + cb;
            if (targetLanguage == "java") return "int " + r + " = " + ca + " % " + cb + ";";
            if (targetLanguage == "javascript") return "let " + r + " = " + ca + " % " + cb + ";";
            return "int " + r + " = " + ca + " % " + cb + ";";
        case Quadruple::OP_LT: return rel("<");
        case Quadruple::OP_GT: return rel(">");
        case Quadruple::OP_LE: return rel("<=");
        case Quadruple::OP_GE: return rel(">=");
        case Quadruple::OP_EQ: return rel("==");
        case Quadruple::OP_NE: return rel("!=");
        case Quadruple::OP_AND: {
            if (targetLanguage == "python") return r + " = int((" + ca + " != 0) and (" + cb + " != 0))";
            if (targetLanguage == "java") return "int " + r + " = ((" + ca + " != 0) && (" + cb + " != 0)) ? 1 : 0;";
            if (targetLanguage == "javascript") return "let " + r + " = ((" + ca + " != 0) && (" + cb + " != 0)) ? 1 : 0;";
            return "int " + r + " = " + ca + " && " + cb + ";";
        }
        case Quadruple::OP_OR: {
            if (targetLanguage == "python") return r + " = int((" + ca + " != 0) or (" + cb + " != 0))";
            if (targetLanguage == "java") return "int " + r + " = ((" + ca + " != 0) || (" + cb + " != 0)) ? 1 : 0;";
            if (targetLanguage == "javascript") return "let " + r + " = ((" + ca + " != 0) || (" + cb + " != 0)) ? 1 : 0;";
            return "int " + r + " = " + ca + " || " + cb + ";";
        }
        default:
            return "";
    }
}

std::string CodeGenerator::getTypeMapping(const std::string& cType) {
    if (targetLanguage == "python") return "";
    if (targetLanguage == "java") {
        if (cType == "int") return "int";
        if (cType == "float") return "double";
        if (cType == "double") return "double";
        if (cType == "char") return "char";
        if (cType == "void") return "void";
        if (cType == "string") return "String";
    }
    if (targetLanguage == "javascript") return "let";
    if (targetLanguage == "cpp") return cType;
    return cType;
}

std::string CodeGenerator::indent(int level) {
    return std::string(level * 4, ' ');
}