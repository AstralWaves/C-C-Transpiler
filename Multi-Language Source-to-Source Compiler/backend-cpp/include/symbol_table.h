#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

enum DataType {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_VOID,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_UNKNOWN
};

struct Symbol {
    std::string name;
    DataType type;
    DataType returnType;  // For functions
    int size;
    int line_declared;
    int scope_level;
    bool is_initialized;
    bool is_function;
    std::string value;
    std::vector<Symbol> parameters;  // For functions
    std::vector<Symbol> members;     // For structs
    
    Symbol() : type(TYPE_UNKNOWN), returnType(TYPE_UNKNOWN), size(0), 
               line_declared(0), scope_level(0), is_initialized(false), 
               is_function(false) {}
};

class SymbolTable {
private:
    std::unordered_map<std::string, Symbol> symbols;
    std::vector<std::string> scope_stack;
    int current_scope;
    
public:
    SymbolTable();
    void enterScope();
    void exitScope();
    bool addSymbol(const std::string& name, DataType type, int line, bool isFunction = false);
    bool addFunction(const std::string& name, DataType returnType, int line);
    Symbol* lookupSymbol(const std::string& name);
    Symbol* lookupSymbolInCurrentScope(const std::string& name);
    void setValue(const std::string& name, const std::string& value);
    int getCurrentScope() const { return current_scope; }
    
    // JSON serialization for frontend
    std::string toJson();
};

#endif