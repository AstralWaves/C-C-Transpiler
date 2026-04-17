#include "../include/symbol_table.h"

SymbolTable::SymbolTable() : current_scope(0) {
    scope_stack.push_back("global");
}

void SymbolTable::enterScope() {
    current_scope++;
    scope_stack.push_back("scope_" + std::to_string(current_scope));
}

void SymbolTable::exitScope() {
    if (!scope_stack.empty()) {
        scope_stack.pop_back();
        current_scope--;
    }
}

bool SymbolTable::addSymbol(const std::string& name, DataType type, int line) {
    if (symbols.find(name) != symbols.end()) return false;
    symbols[name] = {name, type, 0, line, current_scope, false};
    return true;
}

Symbol* SymbolTable::lookupSymbol(const std::string& name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) return &it->second;
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
    // Implement JSON serialization
    return "{}";
}