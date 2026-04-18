/**
 * SEMANTIC ANALYSIS PHASE
 * Responsibility: Checks the "meaning" of the code beyond its structure.
 * It performs type checking, symbol resolution, and scope management.
 */
class SemanticAnalyzer {
    constructor() {
        this.symbolTable = new Map();
        this.currentScope = 'global';
        this.errors = [];
        this.warnings = [];
        this.scopes = ['global'];
        
        // Built-in types recognized by the compiler
        this.builtinTypes = new Set([
            'int', 'float', 'double', 'char', 'void', 'bool', 'long', 'short',
            'unsigned', 'signed', 'auto', 'size_t', 'FILE', 'struct', 'class'
        ]);
        
        // Standard C library functions
        this.builtinFunctions = new Set([
            'printf', 'scanf', 'fopen', 'fclose', 'fread', 'fwrite', 'fseek',
            'remove', 'rename', 'exit', 'malloc', 'free', 'sizeof', 'strcpy',
            'strcmp', 'strlen', 'strcat', 'memset', 'memcpy', 'fprintf', 'sprintf',
            'fgets', 'fputs', 'getchar', 'putchar', 'gets', 'puts'
        ]);
        
        // Built-in constants
        this.builtinConstants = new Set([
            'NULL', 'EOF', 'SEEK_SET', 'SEEK_CUR', 'SEEK_END', 
            'stdin', 'stdout', 'stderr', 'EXIT_SUCCESS', 'EXIT_FAILURE'
        ]);
        
        // Track preprocessor macros
        this.macros = new Map();
        
        // Type compatibility matrix
        this.typeChecking = {
            'int': new Set(['int', 'float', 'double', 'char', 'long', 'short']),
            'float': new Set(['int', 'float', 'double']),
            'double': new Set(['int', 'float', 'double']),
            'char': new Set(['char', 'int']),
            'FILE*': new Set(['FILE*', 'void*']),
            'void*': new Set(['void*', 'FILE*', 'int*', 'char*']),
            'char*': new Set(['char*', 'void*', 'const char*']),
            'const char*': new Set(['const char*', 'char*', 'void*'])
        };
    }
    
    analyze(ast) {
        this.errors = [];
        this.warnings = [];
        this.symbolTable.clear();
        this.scopes = ['global'];
        this.currentScope = 'global';
        this.macros.clear();
        
        // Add built-in types and functions to global scope
        this.addBuiltins();
        
        try {
            this.visitNode(ast);
        } catch (error) {
            this.errors.push({
                phase: 'Semantic',
                message: error.message,
                line: 0,
                column: 0
            });
        }
        
        // Check for unused variables
        this.checkUnusedSymbols();
        
        return {
            symbolTable: this.symbolTable,
            errors: this.errors,
            warnings: this.warnings,
            success: this.errors.length === 0
        };
    }
    
    addBuiltins() {
        // Add standard library functions (marked as builtin)
        this.builtinFunctions.forEach(func => {
            const key = `global:${func}`;
            this.symbolTable.set(key, {
                name: func,
                type: 'function',
                kind: 'builtin',
                scope: 'global',
                line: 0,
                used: true  // Mark as used to avoid warnings
            });
        });
        
        // Add built-in constants (marked as builtin)
        this.builtinConstants.forEach(constant => {
            const key = `global:${constant}`;
            this.symbolTable.set(key, {
                name: constant,
                type: 'int',
                kind: 'builtin',
                scope: 'global',
                line: 0,
                used: true  // Mark as used to avoid warnings
            });
        });
    }
    
    enterScope(scopeName) {
        this.scopes.push(scopeName);
        this.currentScope = scopeName;
    }
    
    exitScope() {
        this.scopes.pop();
        this.currentScope = this.scopes[this.scopes.length - 1] || 'global';
    }
    
    addSymbol(name, type, kind, line) {
        const key = `${this.currentScope}:${name}`;
        
        // Check for redeclaration in same scope
        if (this.symbolTable.has(key)) {
            const existing = this.symbolTable.get(key);
            
            // Function redeclaration is normal in C (prototype then definition)
            if (existing.kind === 'function' && kind === 'function') {
                // Update the existing entry if this one has a body
                if (!existing.hasBody && line > 0) {
                    existing.hasBody = true;
                    existing.line = line;
                }
                // Don't warn about function redeclarations
                return true;
            }
            
            // Don't warn about struct/type redeclarations
            if (kind === 'type' || existing.kind === 'type') {
                return true;
            }
            
            // Skip warning for same symbol in different contexts
            if (existing.kind !== kind) {
                return true;
            }
            
            // Only warn about true redeclarations
            if (existing.kind !== 'builtin' && kind !== 'builtin') {
                this.warnings.push({
                    phase: 'Semantic',
                    message: `Redeclaration of '${name}' in scope '${this.currentScope}'`,
                    line: line,
                    column: 0
                });
                return false;
            }
        }
        
        this.symbolTable.set(key, {
            name: name,
            type: type,
            kind: kind,
            scope: this.currentScope,
            line: line,
            used: kind === 'parameter' || kind === 'builtin', // Parameters are used by definition
            hasBody: line > 0
        });
        
        return true;
    }
    
    lookupSymbol(name) {
        // Check if it's a macro first
        if (this.macros.has(name)) {
            return { type: 'macro', kind: 'macro', name: name, used: true };
        }
        
        // Check if it's a built-in
        if (this.builtinFunctions.has(name)) {
            return { type: 'function', kind: 'builtin', name: name, used: true };
        }
        
        if (this.builtinConstants.has(name)) {
            return { type: 'int', kind: 'builtin', name: name, used: true };
        }
        
        // Search in current and parent scopes
        for (let i = this.scopes.length - 1; i >= 0; i--) {
            const key = `${this.scopes[i]}:${name}`;
            if (this.symbolTable.has(key)) {
                const symbol = this.symbolTable.get(key);
                symbol.used = true;
                return symbol;
            }
        }
        
        return null;
    }
    
    checkTypeCompatibility(type1, type2) {
        if (!type1 || !type2) return true; // Unknown types, assume compatible
        if (type1 === type2) return true;
        
        // Strip qualifiers
        const cleanType1 = type1.replace('const ', '').replace('volatile ', '').trim();
        const cleanType2 = type2.replace('const ', '').replace('volatile ', '').trim();
        
        if (cleanType1 === cleanType2) return true;
        
        // Check compatibility matrix
        if (this.typeChecking[cleanType1]?.has(cleanType2)) return true;
        if (this.typeChecking[cleanType2]?.has(cleanType1)) return true;
        
        // Pointer compatibility
        if (cleanType1.includes('*') && cleanType2.includes('*')) {
            return true;
        }
        
        return false;
    }
    
    checkUnusedSymbols() {
        for (const [key, symbol] of this.symbolTable) {
            // Skip many categories that don't need usage warnings
            if (!symbol.used && 
                symbol.kind !== 'function' && 
                symbol.kind !== 'builtin' &&
                symbol.kind !== 'constant' &&
                symbol.kind !== 'macro' &&
                symbol.kind !== 'parameter' &&
                symbol.kind !== 'type' &&
                symbol.kind !== 'struct' &&
                !symbol.name.startsWith('S_') &&
                !symbol.name.includes('_') &&  // Skip FILE_NAME style macros
                !symbol.name.startsWith('block_')) {  // Skip block scope names
                this.warnings.push({
                    phase: 'Semantic',
                    message: `Variable '${symbol.name}' is declared but never used`,
                    line: symbol.line,
                    column: 0
                });
            }
        }
    }
    
    visitNode(node) {
        if (!node) return null;
        
        switch (node.type) {
            case 'Program':
                return this.visitProgram(node);
            case 'PreprocessorDirective':
                return this.visitPreprocessorDirective(node);
            case 'StructDeclaration':
                return this.visitStructDeclaration(node);
            case 'FunctionDeclaration':
                return this.visitFunctionDeclaration(node);
            case 'BlockStatement':
                return this.visitBlockStatement(node);
            case 'VariableDeclaration':
                return this.visitVariableDeclaration(node);
            case 'ArrayDeclaration':
                return this.visitArrayDeclaration(node);
            case 'IfStatement':
                return this.visitIfStatement(node);
            case 'WhileStatement':
                return this.visitWhileStatement(node);
            case 'DoWhileStatement':
                return this.visitDoWhileStatement(node);
            case 'ForStatement':
                return this.visitForStatement(node);
            case 'SwitchStatement':
                return this.visitSwitchStatement(node);
            case 'SwitchCase':
                return this.visitSwitchCase(node);
            case 'SwitchDefault':
                return this.visitSwitchDefault(node);
            case 'ReturnStatement':
                return this.visitReturnStatement(node);
            case 'BreakStatement':
            case 'ContinueStatement':
                return null;
            case 'ExpressionStatement':
                return this.visitExpressionStatement(node);
            case 'BinaryExpression':
                return this.visitBinaryExpression(node);
            case 'AssignmentExpression':
                return this.visitAssignmentExpression(node);
            case 'LogicalExpression':
                return this.visitLogicalExpression(node);
            case 'UnaryExpression':
                return this.visitUnaryExpression(node);
            case 'UpdateExpression':
                return this.visitUpdateExpression(node);
            case 'CallExpression':
                return this.visitCallExpression(node);
            case 'MemberExpression':
                return this.visitMemberExpression(node);
            case 'ConditionalExpression':
                return this.visitConditionalExpression(node);
            case 'Identifier':
                return this.visitIdentifier(node);
            case 'NumericLiteral':
                return this.visitNumericLiteral(node);
            case 'StringLiteral':
                return 'char*';
            case 'CharLiteral':
                return 'char';
            case 'EmptyStatement':
                return null;
            case 'StructMember':
                return this.visitStructMember(node);
            default:
                // Handle unknown node types gracefully
                if (Array.isArray(node.body)) {
                    node.body.forEach(stmt => this.visitNode(stmt));
                }
                return null;
        }
    }
    
    visitPreprocessorDirective(node) {
        // Parse #define macros
        const value = node.value.trim();
        if (value.startsWith('define')) {
            const parts = value.substring(6).trim().split(/\s+/);
            if (parts.length >= 2) {
                const macroName = parts[0];
                const macroValue = parts.slice(1).join(' ');
                this.macros.set(macroName, macroValue);
                // Don't add to symbol table to avoid unused warnings
            }
        }
        return null;
    }
    
    visitProgram(node) {
        if (Array.isArray(node.body)) {
            node.body.forEach(stmt => this.visitNode(stmt));
        }
        return null;
    }
    
    visitStructDeclaration(node) {
        if (node.name) {
            this.addSymbol(node.name, 'struct', 'type', 0);
        }
        
        // Create a scope for struct members
        if (node.name) {
            this.enterScope(`struct_${node.name}`);
        }
        
        if (node.members) {
            node.members.forEach(member => this.visitNode(member));
        }
        
        if (node.name) {
            this.exitScope();
        }
        
        return 'struct';
    }
    
    visitStructMember(node) {
        if (node.declarators) {
            node.declarators.forEach(decl => {
                if (decl.name) {
                    this.addSymbol(decl.name, decl.type || 'int', 'member', 0);
                }
            });
        }
        return null;
    }
    
    visitFunctionDeclaration(node) {
        const returnType = node.returnType || 'void';
        
        // Add function to symbol table
        this.addSymbol(node.name, returnType, 'function', 0);
        
        // Create function scope
        this.enterScope(node.name);
        
        // Add parameters to symbol table
        if (node.params) {
            node.params.forEach(param => {
                if (param.name) {
                    this.addSymbol(param.name, param.type || 'int', 'parameter', 0);
                }
            });
        }
        
        // Visit function body
        if (node.body && Array.isArray(node.body)) {
            node.body.forEach(stmt => this.visitNode(stmt));
        }
        
        this.exitScope();
        
        return returnType;
    }
    
    visitBlockStatement(node) {
        this.enterScope(`block_${Date.now()}_${Math.random()}`);
        
        if (Array.isArray(node.body)) {
            node.body.forEach(stmt => this.visitNode(stmt));
        }
        
        this.exitScope();
        return null;
    }
    
    visitVariableDeclaration(node) {
        const varType = node.kind || 'int';
        let initType = null;
        
        if (node.value) {
            initType = this.visitNode(node.value);
            
            if (initType && !this.checkTypeCompatibility(varType, initType)) {
                // Special case: FILE* = fopen(...)
                if (!(varType.includes('FILE') && initType.includes('FILE'))) {
                    this.warnings.push({
                        phase: 'Semantic',
                        message: `Type mismatch: cannot assign '${initType}' to '${varType}'`,
                        line: 0,
                        column: 0
                    });
                }
            }
        }
        
        this.addSymbol(node.name, varType, 'variable', 0);
        
        return varType;
    }
    
    visitArrayDeclaration(node) {
        const varType = `${node.elementType}[]`;
        this.addSymbol(node.name, varType, 'variable', 0);
        return varType;
    }
    
    visitIfStatement(node) {
        const testType = this.visitNode(node.test);
        
        if (testType && testType !== 'function' && !this.checkTypeCompatibility(testType, 'int')) {
            this.warnings.push({
                phase: 'Semantic',
                message: `Condition should be scalar type, got '${testType}'`,
                line: 0,
                column: 0
            });
        }
        
        if (node.consequent) {
            node.consequent.forEach(stmt => this.visitNode(stmt));
        }
        
        if (node.alternate) {
            node.alternate.forEach(stmt => this.visitNode(stmt));
        }
        
        return null;
    }
    
    visitWhileStatement(node) {
        const testType = this.visitNode(node.test);
        
        // Skip warning for function calls in condition (like while(fread(...)))
        if (testType && testType !== 'function' && !this.checkTypeCompatibility(testType, 'int')) {
            this.warnings.push({
                phase: 'Semantic',
                message: `Loop condition should be scalar type, got '${testType}'`,
                line: 0,
                column: 0
            });
        }
        
        if (node.body) {
            node.body.forEach(stmt => this.visitNode(stmt));
        }
        
        return null;
    }
    
    visitDoWhileStatement(node) {
        const testType = this.visitNode(node.test);
        
        if (node.body) {
            node.body.forEach(stmt => this.visitNode(stmt));
        }
        
        return null;
    }
    
    visitForStatement(node) {
        if (node.init) this.visitNode(node.init);
        if (node.test) this.visitNode(node.test);
        if (node.update) this.visitNode(node.update);
        
        if (node.body) {
            node.body.forEach(stmt => this.visitNode(stmt));
        }
        
        return null;
    }
    
    visitSwitchStatement(node) {
        const discriminantType = this.visitNode(node.discriminant);
        
        if (node.cases) {
            node.cases.forEach(caseStmt => this.visitNode(caseStmt));
        }
        
        return null;
    }
    
    visitSwitchCase(node) {
        if (node.test) this.visitNode(node.test);
        if (node.consequent) {
            node.consequent.forEach(stmt => this.visitNode(stmt));
        }
        return null;
    }
    
    visitSwitchDefault(node) {
        if (node.consequent) {
            node.consequent.forEach(stmt => this.visitNode(stmt));
        }
        return null;
    }
    
    visitReturnStatement(node) {
        if (node.value) {
            return this.visitNode(node.value);
        }
        return 'void';
    }
    
    visitExpressionStatement(node) {
        return this.visitNode(node.expression);
    }
    
    visitBinaryExpression(node) {
        const leftType = this.visitNode(node.left);
        const rightType = this.visitNode(node.right);
        
        // Type promotion
        if (leftType === 'double' || rightType === 'double') return 'double';
        if (leftType === 'float' || rightType === 'float') return 'float';
        if (leftType === 'int' || rightType === 'int') return 'int';
        
        return leftType || rightType || 'int';
    }
    
    visitAssignmentExpression(node) {
        const leftType = this.visitNode(node.left);
        const rightType = this.visitNode(node.right);
        
        if (leftType && rightType && !this.checkTypeCompatibility(leftType, rightType)) {
            // Special cases for pointer assignments
            if (!(leftType.includes('*') && rightType.includes('*'))) {
                this.warnings.push({
                    phase: 'Semantic',
                    message: `Type mismatch in assignment: '${rightType}' to '${leftType}'`,
                    line: 0,
                    column: 0
                });
            }
        }
        
        return leftType;
    }
    
    visitLogicalExpression(node) {
        this.visitNode(node.left);
        this.visitNode(node.right);
        return 'int';
    }
    
    visitUnaryExpression(node) {
        const argType = this.visitNode(node.argument);
        
        // Address-of operator returns pointer
        if (node.operator === '&') {
            return argType + '*';
        }
        
        // Dereference operator returns pointed type
        if (node.operator === '*') {
            if (argType && argType.endsWith('*')) {
                return argType.slice(0, -1);
            }
            return 'int';
        }
        
        return argType;
    }
    
    visitUpdateExpression(node) {
        return this.visitNode(node.argument);
    }
    
    visitCallExpression(node) {
        // Get callee name
        let calleeName = null;
        if (typeof node.callee === 'string') {
            calleeName = node.callee;
        } else if (node.callee && node.callee.type === 'Identifier') {
            calleeName = node.callee.name;
        } else if (node.callee && node.callee.name) {
            calleeName = node.callee.name;
        }
        
        // Check if function exists
        if (calleeName) {
            const symbol = this.lookupSymbol(calleeName);
            if (!symbol) {
                // Only warn for non-builtin functions
                if (!this.builtinFunctions.has(calleeName)) {
                    // Add it implicitly (common in C)
                    this.addSymbol(calleeName, 'int', 'function', 0);
                }
            }
            
            // Visit arguments
            if (node.arguments) {
                node.arguments.forEach(arg => this.visitNode(arg));
            }
            
            // Return type based on function name heuristics
            if (calleeName.includes('fopen')) {
                return 'FILE*';
            }
            if (calleeName === 'printf' || calleeName === 'scanf') {
                return 'int';
            }
            
            return symbol?.type || 'int';
        }
        
        return 'int';
    }
    
    visitMemberExpression(node) {
        const objectType = this.visitNode(node.object);
        
        if (node.computed) {
            this.visitNode(node.property);
        }
        
        // Return the type of the member (simplified)
        if (objectType === 'struct' || objectType === 'FILE*') {
            return 'int';
        }
        
        return 'int';
    }
    
    visitConditionalExpression(node) {
        this.visitNode(node.test);
        const consequentType = this.visitNode(node.consequent);
        const alternateType = this.visitNode(node.alternate);
        
        // Return common type
        return consequentType || alternateType || 'int';
    }
    
    visitIdentifier(node) {
        const name = node.name;
        
        // Check if it's a macro
        if (this.macros.has(name)) {
            return 'macro';
        }
        
        const symbol = this.lookupSymbol(name);
        
        if (!symbol) {
            // Don't report for common patterns (single letters, known patterns)
            if (!this.builtinFunctions.has(name) && 
                !this.builtinConstants.has(name) &&
                !name.startsWith('S_') &&  // SEEK_SET, etc.
                !name.includes('_') &&     // FILE_NAME, etc.
                name.length > 1) {         // Single letters often used in loops
                // Assume it's an int variable (common in C)
                this.addSymbol(name, 'int', 'variable', 0);
            }
            return 'int';
        }
        
        return symbol.type;
    }
    
    visitNumericLiteral(node) {
        const value = node.value;
        if (value.includes('.')) {
            return 'double';
        }
        return 'int';
    }
    
    formatSymbolTable() {
        if (this.symbolTable.size === 0) {
            return 'Symbol Table is empty';
        }
        
        let result = '📊 Symbol Table\n';
        result += '━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n';
        result += 'Scope'.padEnd(20) + 'Name'.padEnd(15) + 'Type'.padEnd(12) + 'Kind'.padEnd(10) + 'Used\n';
        result += '──────────────────────────────────────────────────────────────────\n';
        
        // Group by scope
        const byScope = {};
        for (const [key, symbol] of this.symbolTable) {
            // Skip builtins for cleaner display
            if (symbol.kind === 'builtin') continue;
            
            if (!byScope[symbol.scope]) {
                byScope[symbol.scope] = [];
            }
            byScope[symbol.scope].push(symbol);
        }
        
        // Sort scopes (global first, then functions, then blocks)
        const scopes = Object.keys(byScope).sort((a, b) => {
            if (a === 'global') return -1;
            if (b === 'global') return 1;
            return a.localeCompare(b);
        });
        
        for (const scope of scopes) {
            // Truncate long scope names
            const scopeDisplay = scope.length > 18 ? scope.substring(0, 15) + '...' : scope;
            result += `\n[${scopeDisplay}]\n`;
            const symbols = byScope[scope].sort((a, b) => a.name.localeCompare(b.name));
            
            for (const symbol of symbols) {
                const usedMark = symbol.used ? '✓' : ' ';
                const scopeCol = ''.padEnd(2);
                const nameCol = symbol.name.padEnd(15);
                const typeCol = (symbol.type || 'unknown').padEnd(12);
                const kindCol = symbol.kind.padEnd(10);
                result += `${scopeCol}${nameCol} ${typeCol} ${kindCol} ${usedMark}\n`;
            }
        }
        
        // Add macros section
        if (this.macros.size > 0) {
            result += '\n[Macros]\n';
            for (const [name, value] of this.macros) {
                const displayValue = value.length > 30 ? value.substring(0, 27) + '...' : value;
                result += `  #define ${name} ${displayValue}\n`;
            }
        }
        
        result += '\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n';
        result += `Total Symbols: ${this.symbolTable.size - this.builtinFunctions.size - this.builtinConstants.size}`;
        
        return result;
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = SemanticAnalyzer;
}