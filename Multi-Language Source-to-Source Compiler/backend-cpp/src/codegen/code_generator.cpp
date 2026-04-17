#include "../include/code_generator.h"
#include <sstream>
#include <algorithm>

CodeGenerator::CodeGenerator(SymbolTable& symTable, const std::string& target)
    : symbolTable(symTable), targetLanguage(target) {}

std::string CodeGenerator::generate(const std::vector<Quadruple>& intermediateCode) {
    if (targetLanguage == "python") {
        return generatePython(intermediateCode);
    } else if (targetLanguage == "java") {
        return generateJava(intermediateCode);
    } else if (targetLanguage == "javascript") {
        return generateJavaScript(intermediateCode);
    }
    return "";
}

std::string CodeGenerator::generatePython(const std::vector<Quadruple>& code) {
    std::ostringstream output;
    int indentLevel = 0;
    
    output << "# Generated Python code from C/C++ source\n";
    output << "# Converted by C-Transpiler\n\n";
    
    for (const auto& quad : code) {
        std::string line;
        
        switch (quad.op) {
            case Quadruple::OP_ASSIGN:
                line = quad.result + " = " + quad.arg1;
                break;
                
            case Quadruple::OP_ADD:
                line = quad.result + " = " + quad.arg1 + " + " + quad.arg2;
                break;
                
            case Quadruple::OP_SUB:
                line = quad.result + " = " + quad.arg1 + " - " + quad.arg2;
                break;
                
            case Quadruple::OP_MUL:
                line = quad.result + " = " + quad.arg1 + " * " + quad.arg2;
                break;
                
            case Quadruple::OP_DIV:
                line = quad.result + " = " + quad.arg1 + " / " + quad.arg2;
                break;
                
            case Quadruple::OP_IF:
                line = "if " + quad.arg1 + ":";
                break;
                
            case Quadruple::OP_ELSE:
                indentLevel--;
                output << indent(indentLevel) << "else:\n";
                indentLevel++;
                continue;
                
            case Quadruple::OP_WHILE:
                line = "while " + quad.arg1 + ":";
                break;
                
            case Quadruple::OP_FOR:
                line = "for " + quad.arg1 + ":";
                break;
                
            case Quadruple::OP_PRINTF:
                line = "print(" + quad.arg1 + ")";
                break;
                
            case Quadruple::OP_SCANF:
                line = quad.arg1 + " = input()";
                break;
                
            case Quadruple::OP_RETURN:
                if (quad.arg1.empty()) {
                    line = "return";
                } else {
                    line = "return " + quad.arg1;
                }
                break;
                
            case Quadruple::OP_FUNC_START:
                output << "\n" << indent(indentLevel) << "def " + quad.result + "(" + quad.arg1 + "):\n";
                indentLevel++;
                continue;
                
            case Quadruple::OP_FUNC_END:
                indentLevel--;
                continue;
                
            case Quadruple::OP_CALL:
                line = quad.result + " = " + quad.arg1 + "(" + quad.arg2 + ")";
                break;
                
            default:
                continue;
        }
        
        if (!line.empty()) {
            output << indent(indentLevel) << line << "\n";
            if (quad.op == Quadruple::OP_IF || quad.op == Quadruple::OP_WHILE || quad.op == Quadruple::OP_FOR) {
                indentLevel++;
            }
        }
    }
    
    return output.str();
}

std::string CodeGenerator::generateJava(const std::vector<Quadruple>& code) {
    std::ostringstream output;
    int indentLevel = 1;
    bool inMain = false;
    
    output << "// Generated Java code from C/C++ source\n";
    output << "// Converted by C-Transpiler\n\n";
    output << "public class GeneratedCode {\n";
    
    for (const auto& quad : code) {
        std::string line;
        
        switch (quad.op) {
            case Quadruple::OP_ASSIGN:
                line = quad.result + " = " + quad.arg1 + ";";
                break;
                
            case Quadruple::OP_ADD:
                line = quad.result + " = " + quad.arg1 + " + " + quad.arg2 + ";";
                break;
                
            case Quadruple::OP_SUB:
                line = quad.result + " = " + quad.arg1 + " - " + quad.arg2 + ";";
                break;
                
            case Quadruple::OP_MUL:
                line = quad.result + " = " + quad.arg1 + " * " + quad.arg2 + ";";
                break;
                
            case Quadruple::OP_DIV:
                line = quad.result + " = " + quad.arg1 + " / " + quad.arg2 + ";";
                break;
                
            case Quadruple::OP_IF:
                line = "if (" + quad.arg1 + ") {";
                indentLevel++;
                break;
                
            case Quadruple::OP_ELSE:
                indentLevel--;
                line = "} else {";
                indentLevel++;
                break;
                
            case Quadruple::OP_WHILE:
                line = "while (" + quad.arg1 + ") {";
                indentLevel++;
                break;
                
            case Quadruple::OP_FOR:
                line = "for (" + quad.arg1 + ") {";
                indentLevel++;
                break;
                
            case Quadruple::OP_PRINTF:
                line = "System.out.println(" + quad.arg1 + ");";
                break;
                
            case Quadruple::OP_RETURN:
                if (quad.arg1.empty()) {
                    line = "return;";
                } else {
                    line = "return " + quad.arg1 + ";";
                }
                break;
                
            case Quadruple::OP_FUNC_START:
                if (quad.result == "main") {
                    line = "public static void main(String[] args) {";
                    inMain = true;
                } else {
                    std::string params = "";
                    if (!quad.arg1.empty()) {
                        std::stringstream ss(quad.arg1);
                        std::string param;
                        while (std::getline(ss, param, ',')) {
                            if (!params.empty()) params += ", ";
                            params += "int " + param;
                        }
                    }
                    line = "public static int " + quad.result + "(" + params + ") {";
                }
                indentLevel++;
                break;
                
            case Quadruple::OP_FUNC_END:
                indentLevel--;
                line = "}";
                break;

            case Quadruple::OP_CALL:
                line = "int " + quad.result + " = " + quad.arg1 + "(" + quad.arg2 + ");";
                break;
                
            default:
                continue;
        }
        
        if (!line.empty()) {
            if (line == "}") {
                output << indent(indentLevel) << line << "\n";
            } else {
                output << indent(indentLevel) << line << "\n";
            }
        }
    }
    
    output << "}\n";
    return output.str();
}

std::string CodeGenerator::generateJavaScript(const std::vector<Quadruple>& code) {
    std::ostringstream output;
    int indentLevel = 0;
    
    output << "// Generated JavaScript code from C/C++ source\n";
    output << "// Converted by C-Transpiler\n\n";
    
    for (const auto& quad : code) {
        std::string line;
        
        switch (quad.op) {
            case Quadruple::OP_ASSIGN:
                line = "let " + quad.result + " = " + quad.arg1 + ";";
                break;
                
            case Quadruple::OP_ADD:
                line = "let " + quad.result + " = " + quad.arg1 + " + " + quad.arg2 + ";";
                break;
                
            case Quadruple::OP_SUB:
                line = "let " + quad.result + " = " + quad.arg1 + " - " + quad.arg2 + ";";
                break;
                
            case Quadruple::OP_MUL:
                line = "let " + quad.result + " = " + quad.arg1 + " * " + quad.arg2 + ";";
                break;
                
            case Quadruple::OP_DIV:
                line = "let " + quad.result + " = " + quad.arg1 + " / " + quad.arg2 + ";";
                break;
                
            case Quadruple::OP_IF:
                line = "if (" + quad.arg1 + ") {";
                indentLevel++;
                break;
                
            case Quadruple::OP_ELSE:
                indentLevel--;
                line = "} else {";
                indentLevel++;
                break;
                
            case Quadruple::OP_WHILE:
                line = "while (" + quad.arg1 + ") {";
                indentLevel++;
                break;
                
            case Quadruple::OP_FOR:
                line = "for (" + quad.arg1 + ") {";
                indentLevel++;
                break;
                
            case Quadruple::OP_PRINTF:
                line = "console.log(" + quad.arg1 + ");";
                break;
                
            case Quadruple::OP_RETURN:
                if (quad.arg1.empty()) {
                    line = "return;";
                } else {
                    line = "return " + quad.arg1 + ";";
                }
                break;
                
            case Quadruple::OP_FUNC_START:
                line = "function " + quad.result + "(" + quad.arg1 + ") {";
                indentLevel++;
                break;
                
            case Quadruple::OP_FUNC_END:
                indentLevel--;
                line = "}";
                break;
                
            case Quadruple::OP_CALL:
                line = "let " + quad.result + " = " + quad.arg1 + "(" + quad.arg2 + ");";
                break;
                
            default:
                continue;
        }
        
        if (!line.empty()) {
            if (line == "}" || line == "} else {") {
                output << indent(indentLevel) << line << "\n";
            } else {
                output << indent(indentLevel) << line << "\n";
            }
        }
    }
    
    return output.str();
}

std::string CodeGenerator::indent(int level) {
    return std::string(level * 4, ' ');
}