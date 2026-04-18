#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include "symbol_table.h"
#include "intermediate_code.h"

class CodeGenerator {
private:
    SymbolTable& symbolTable;
    std::string targetLanguage;
    std::string output;
    std::vector<std::string> generatedCode;
    
    // Helper methods
    std::string generatePython(const std::vector<Quadruple>& code);
    std::string generateJava(const std::vector<Quadruple>& code);
    std::string generateJavaScript(const std::vector<Quadruple>& code);
    std::string generateCpp(const std::vector<Quadruple>& code);
    
    std::string convertExpression(const std::string& expr);
    std::string formatPrintfString(const std::string& format);
    std::string formatPrintfStringUtilJs(const std::string& format);
    std::string convertPrintfToJava(const std::string& format);
    std::string convertPrintfToJs(const std::string& format);
    std::string emitOperatorQuad(const Quadruple& quad);
    std::string javaIntCondition(const std::string& expr);
    std::string jsEmitPrintf(const Quadruple& quad);
    std::string getTypeMapping(const std::string& cType);
    std::string indent(int level);
    
public:
    CodeGenerator(SymbolTable& symTable, const std::string& target);
    std::string generate(const std::vector<Quadruple>& intermediateCode);
};

#endif