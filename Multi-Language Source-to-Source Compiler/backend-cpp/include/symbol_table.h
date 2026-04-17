#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <unordered_map>
#include <string>
#include <vector>

enum DataType {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_VOID,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_STRUCT
};

struct Symbol {
    std::string name;
    DataType type;
    int size;
    int line_declared;
    int scope_level;
    bool is_initialized;
    std::string value;
    std::vector<Symbol> members;  // For structs
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
    bool addSymbol(const std::string& name, DataType type, int line);
    Symbol* lookupSymbol(const std::string& name);
    void setValue(const std::string& name, const std::string& value);
    
    // JSON serialization for frontend
    std::string toJson();
};

#endif