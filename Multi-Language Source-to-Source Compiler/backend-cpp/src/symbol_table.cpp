#include "../include/symbol_table.h"
#include <sstream>

SymbolTable::SymbolTable() : current_scope(0) {
    scope_stack.push_back("global");
}

void SymbolTable::enterScope() {
    current_scope++;
    scope_stack.push_back("scope_" + std::to_string(current_scope));
}

void SymbolTable::exitScope() {
    if (!scope_stack.empty()) {
        // Remove all symbols in current scope
        std::string currentScope = scope_stack.back();
        for (auto it = symbols.begin(); it != symbols.end();) {
            if (it->second.scope_level == current_scope) {
                it = symbols.erase(it);
            } else {
                ++it;
            }
        }
        scope_stack.pop_back();
        current_scope--;
    }
}

bool SymbolTable::addSymbol(const std::string& name, DataType type, int line, bool isFunction) {
    // Check if symbol already exists in current scope
    for (const auto& pair : symbols) {
        if (pair.second.name == name && pair.second.scope_level == current_scope) {
            return false;
        }
    }
    
    Symbol sym;
    sym.name = name;
    sym.type = type;
    sym.line_declared = line;
    sym.scope_level = current_scope;
    sym.is_function = isFunction;
    sym.is_initialized = false;
    symbols[name] = sym;
    return true;
}

bool SymbolTable::addFunction(const std::string& name, DataType returnType, int line) {
    Symbol sym;
    sym.name = name;
    sym.returnType = returnType;
    sym.type = TYPE_VOID;
    sym.line_declared = line;
    sym.scope_level = current_scope;
    sym.is_function = true;
    sym.is_initialized = true;
    symbols[name] = sym;
    return true;
}

Symbol* SymbolTable::lookupSymbol(const std::string& name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second;
    }
    return nullptr;
}

Symbol* SymbolTable::lookupSymbolInCurrentScope(const std::string& name) {
    for (auto& pair : symbols) {
        if (pair.second.name == name && pair.second.scope_level == current_scope) {
            return &pair.second;
        }
    }
    return nullptr;
}

void SymbolTable::setValue(const std::string& name, const std::string& value) {
    auto sym = lookupSymbol(name);
    if (sym) {
        sym->value = value;
        sym->is_initialized = true;
    }
}

std::string SymbolTable::toJson() {
    std::stringstream ss;
    ss << "[";
    bool first = true;
    for (const auto& pair : symbols) {
        if (!first) ss << ",";
        first = false;
        ss << "{";
        ss << "\"name\":\"" << pair.second.name << "\",";
        ss << "\"scope\":\"" << scope_stack[pair.second.scope_level] << "\",";
        ss << "\"line\":" << pair.second.line_declared;
        ss << "}";
    }
    ss << "]";
    return ss.str();
}