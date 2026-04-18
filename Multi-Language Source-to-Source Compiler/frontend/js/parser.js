/**
 * SYNTAX ANALYSIS PHASE (PARSER)
 * Responsibility: Takes the list of tokens and builds an Abstract Syntax Tree (AST).
 * It verifies that the code follows the grammatical rules (e.g., matching parentheses).
 */
class Parser {
    constructor() {
        this.tokens = [];
        this.position = 0;
        this.errors = [];
        this.warnings = [];
        this.typeDefinitions = new Set(); // Track defined types
    }

    /**
     * Entry point for the parser. 
     * It transforms tokens into a Program node containing a body of statements.
     */
    parse(tokens) {
        // Filter out EOF tokens and keep preprocessor directives for reference
        this.tokens = tokens.filter(t => t.type !== 'EOF');
        this.position = 0;
        this.errors = [];
        this.warnings = [];
        this.typeDefinitions.clear();
        
        const body = [];
        
        try {
            while (this.position < this.tokens.length) {
                const stmt = this.parseTopLevel();
                if (stmt) body.push(stmt);
            }
            
            return {
                success: this.errors.length === 0,
                ast: { type: 'Program', body: body },
                errors: this.errors,
                warnings: this.warnings
            };
        } catch (error) {
            console.error('Parse error:', error);
            
            this.errors.push({
                phase: 'Syntax',
                message: error.message,
                line: this.getCurrentToken()?.line || 0,
                column: this.getCurrentToken()?.column || 0
            });
            
            return {
                success: false,
                ast: { type: 'Program', body: body },
                errors: this.errors,
                warnings: this.warnings
            };
        }
    }

    getCurrentToken() {
        return this.tokens[this.position];
    }
    
    consume() {
        return this.tokens[this.position++];
    }
    
    peek(offset = 0) {
        return this.tokens[this.position + offset];
    }

    isTypeToken(token) {
        if (!token) return false;
        const commonTypes = [
            'int', 'float', 'double', 'char', 'void', 'bool', 'long', 'short', 
            'unsigned', 'signed', 'auto', 'struct', 'class', 'enum',
            'FILE', 'size_t', 'uint8_t', 'uint16_t', 'uint32_t', 'int32_t'
        ];
        return (token.type === 'KEYWORD' && commonTypes.includes(token.value)) ||
               (token.type === 'IDENTIFIER' && commonTypes.includes(token.value)) ||
               this.typeDefinitions.has(token.value);
    }

    parseTopLevel() {
        const token = this.getCurrentToken();
        if (!token) return null;

        // Skip preprocessor directives
        if (token.type === 'PREPROCESSOR') {
            const directive = this.consume().value;
            return { type: 'PreprocessorDirective', value: directive };
        }

        // Handle struct definition
        if (token.type === 'KEYWORD' && token.value === 'struct') {
            return this.parseStruct();
        }

        // Handle function declarations and definitions
        if (this.isTypeToken(token)) {
            return this.parseFunctionOrDeclaration();
        }

        // Skip any unknown token to prevent infinite loops
        console.warn('Skipping unexpected token at top level:', token);
        this.consume();
        return null;
    }

    parseStruct() {
        this.consume(); // 'struct'
        
        let name = null;
        if (this.getCurrentToken().type === 'IDENTIFIER') {
            name = this.consume().value;
            this.typeDefinitions.add(name);
        }
        
        // Check if it's just a forward declaration
        if (this.getCurrentToken().value === ';') {
            this.consume();
            return {
                type: 'StructDeclaration',
                name: name,
                forward: true
            };
        }
        
        // Parse struct body
        this.expect('PUNCTUATION', '{');
        const members = [];
        
        while (this.getCurrentToken() && this.getCurrentToken().value !== '}') {
            const member = this.parseStructMember();
            if (member) members.push(member);
        }
        
        this.expect('PUNCTUATION', '}');
        
        // Handle variables declared after struct
        const variables = [];
        while (this.getCurrentToken() && this.getCurrentToken().value !== ';') {
            if (this.getCurrentToken().type === 'IDENTIFIER') {
                variables.push(this.consume().value);
                if (this.getCurrentToken().value === ',') {
                    this.consume();
                }
            } else {
                break;
            }
        }
        
        if (this.getCurrentToken().value === ';') {
            this.consume();
        }
        
        return {
            type: 'StructDeclaration',
            name: name,
            members: members,
            variables: variables
        };
    }

    parseStructMember() {
        // Parse type
        const memberType = this.parseType();
        
        // Parse declarators
        const declarators = [];
        
        do {
            let name = null;
            let pointerCount = 0;
            
            while (this.getCurrentToken().value === '*') {
                this.consume();
                pointerCount++;
            }
            
            if (this.getCurrentToken().type === 'IDENTIFIER') {
                name = this.consume().value;
            }
            
            // Array size
            let arraySize = null;
            if (this.getCurrentToken().value === '[') {
                this.consume();
                if (this.getCurrentToken().type === 'INTEGER') {
                    arraySize = this.consume().value;
                }
                this.expect('PUNCTUATION', ']');
            }
            
            if (name) {
                declarators.push({ name, type: memberType, pointerCount, arraySize });
            }
            
            if (this.getCurrentToken().value === ',') {
                this.consume();
            } else {
                break;
            }
        } while (true);
        
        this.expect('PUNCTUATION', ';');
        
        return { type: 'StructMember', declarators };
    }

    parseFunctionOrDeclaration() {
        const returnType = this.parseType();
        
        // Handle multiple declarators (e.g., int x, y, z;)
        if (this.getCurrentToken().type === 'IDENTIFIER') {
            const name = this.getCurrentToken().value;
            const nextToken = this.peek(1);
            
            // Check if it's a function (has parentheses)
            if (nextToken && nextToken.value === '(') {
                // Go back to parse as function
                this.position -= returnType.split(' ').length;
                return this.parseFunction();
            } else {
                // Parse as variable declaration
                const declarations = [];
                const baseType = returnType;
                
                do {
                    let varName = this.expect('IDENTIFIER').value;
                    let init = null;
                    
                    // Array
                    if (this.getCurrentToken().value === '[') {
                        this.consume();
                        let size = null;
                        if (this.getCurrentToken().type === 'INTEGER') {
                            size = this.consume().value;
                        }
                        this.expect('PUNCTUATION', ']');
                        
                        if (this.getCurrentToken().value === '=') {
                            this.consume();
                            init = this.parseExpression();
                        }
                        
                        declarations.push({
                            type: 'ArrayDeclaration',
                            elementType: baseType,
                            name: varName,
                            size: size,
                            init: init
                        });
                    } else {
                        if (this.getCurrentToken().value === '=') {
                            this.consume();
                            init = this.parseExpression();
                        }
                        
                        declarations.push({
                            type: 'VariableDeclaration',
                            kind: baseType,
                            name: varName,
                            value: init
                        });
                    }
                    
                    if (this.getCurrentToken().value === ',') {
                        this.consume();
                    } else {
                        break;
                    }
                } while (true);
                
                this.expect('PUNCTUATION', ';');
                
                return declarations.length === 1 ? declarations[0] : {
                    type: 'BlockStatement',
                    body: declarations
                };
            }
        }
        
        // Should not reach here
        throw new Error('Expected identifier');
    }

    parseFunction() {
        const returnType = this.parseType();
        const name = this.expect('IDENTIFIER').value;
        
        this.expect('PUNCTUATION', '(');
        const params = this.parseParameters();
        this.expect('PUNCTUATION', ')');
        
        // Check if it's a prototype or definition
        let body = null;
        if (this.getCurrentToken() && this.getCurrentToken().value === '{') {
            const block = this.parseBlock();
            body = block.body;
        } else {
            this.expect('PUNCTUATION', ';');
        }
        
        return {
            type: 'FunctionDeclaration',
            returnType: returnType,
            name: name,
            params: params,
            body: body
        };
    }

    parseType() {
        const parts = [];
        const typeKeywords = [
            'int', 'float', 'double', 'char', 'void', 'bool', 'auto',
            'FILE', 'size_t', 'uint8_t', 'uint16_t', 'uint32_t', 'int32_t'
        ];
        
        while (this.getCurrentToken()) {
            const token = this.getCurrentToken();
            
            if (token.type === 'KEYWORD' && 
                ['const', 'volatile', 'static', 'unsigned', 'signed', 'long', 'short'].includes(token.value)) {
                parts.push(this.consume().value);
            } else if ((token.type === 'KEYWORD' || token.type === 'IDENTIFIER') && 
                       typeKeywords.includes(token.value)) {
                parts.push(this.consume().value);
                break;
            } else if (token.value === 'struct') {
                parts.push(this.consume().value);
                if (this.getCurrentToken().type === 'IDENTIFIER') {
                    parts.push(this.consume().value);
                }
                break;
            } else if (this.typeDefinitions.has(token.value)) {
                parts.push(this.consume().value);
                break;
            } else {
                break;
            }
        }
        
        // Handle pointers
        while (this.getCurrentToken() && this.getCurrentToken().value === '*') {
            parts.push(this.consume().value);
        }
        
        return parts.join(' ');
    }

    parseParameters() {
        const params = [];
        
        if (this.getCurrentToken().value === ')') {
            return params;
        }
        
        if (this.getCurrentToken().value === 'void' && this.peek(1).value === ')') {
            this.consume();
            return params;
        }
        
        while (this.getCurrentToken() && this.getCurrentToken().value !== ')') {
            const paramType = this.parseType();
            let paramName = '';
            
            if (this.getCurrentToken().type === 'IDENTIFIER') {
                paramName = this.consume().value;
            }
            
            params.push({ type: paramType, name: paramName });
            
            if (this.getCurrentToken().value === ',') {
                this.consume();
            } else {
                break;
            }
        }
        
        return params;
    }

    parseBlock() {
        this.expect('PUNCTUATION', '{');
        const body = [];
        
        while (this.getCurrentToken() && this.getCurrentToken().value !== '}') {
            const stmt = this.parseStatement();
            if (stmt) body.push(stmt);
        }
        
        this.expect('PUNCTUATION', '}');
        
        return { type: 'BlockStatement', body };
    }

    parseStatement() {
        const token = this.getCurrentToken();
        if (!token) return null;

        // Block
        if (token.value === '{') {
            return this.parseBlock();
        }

        // Variable declaration
        if (this.isTypeToken(token)) {
            return this.parseFunctionOrDeclaration();
        }

        // Keywords
        if (token.type === 'KEYWORD') {
            switch (token.value) {
                case 'if': return this.parseIf();
                case 'while': return this.parseWhile();
                case 'for': return this.parseFor();
                case 'do': return this.parseDoWhile();
                case 'switch': return this.parseSwitch();
                case 'return': return this.parseReturn();
                case 'break': 
                    this.consume();
                    this.optionalSemicolon();
                    return { type: 'BreakStatement' };
                case 'continue':
                    this.consume();
                    this.optionalSemicolon();
                    return { type: 'ContinueStatement' };
            }
        }

        // Empty statement
        if (token.value === ';') {
            this.consume();
            return { type: 'EmptyStatement' };
        }

        // Expression statement
        return this.parseExpressionStatement();
    }

    parseIf() {
        this.expect('KEYWORD', 'if');
        this.expect('PUNCTUATION', '(');
        const test = this.parseExpression();
        this.expect('PUNCTUATION', ')');
        
        const consequent = this.parseStatement();
        let alternate = null;
        
        if (this.getCurrentToken() && this.getCurrentToken().value === 'else') {
            this.consume();
            alternate = this.parseStatement();
        }
        
        return {
            type: 'IfStatement',
            test: test,
            consequent: this.toArray(consequent),
            alternate: alternate ? this.toArray(alternate) : null
        };
    }

    parseWhile() {
        this.expect('KEYWORD', 'while');
        this.expect('PUNCTUATION', '(');
        const test = this.parseExpression();
        this.expect('PUNCTUATION', ')');
        
        const body = this.parseStatement();
        
        return {
            type: 'WhileStatement',
            test: test,
            body: this.toArray(body)
        };
    }

    parseDoWhile() {
        this.expect('KEYWORD', 'do');
        const body = this.parseStatement();
        this.expect('KEYWORD', 'while');
        this.expect('PUNCTUATION', '(');
        const test = this.parseExpression();
        this.expect('PUNCTUATION', ')');
        this.optionalSemicolon();
        
        return {
            type: 'DoWhileStatement',
            test: test,
            body: this.toArray(body)
        };
    }

    parseFor() {
        this.expect('KEYWORD', 'for');
        this.expect('PUNCTUATION', '(');
        
        let init = null;
        if (this.getCurrentToken().value !== ';') {
            if (this.isTypeToken(this.getCurrentToken())) {
                init = this.parseFunctionOrDeclaration();
            } else {
                init = this.parseExpression();
                this.expect('PUNCTUATION', ';');
            }
        } else {
            this.expect('PUNCTUATION', ';');
        }
        
        let test = null;
        if (this.getCurrentToken().value !== ';') {
            test = this.parseExpression();
        }
        this.expect('PUNCTUATION', ';');
        
        let update = null;
        if (this.getCurrentToken().value !== ')') {
            update = this.parseExpression();
        }
        this.expect('PUNCTUATION', ')');
        
        const body = this.parseStatement();
        
        return {
            type: 'ForStatement',
            init: init,
            test: test,
            update: update,
            body: this.toArray(body)
        };
    }

    parseSwitch() {
        this.expect('KEYWORD', 'switch');
        this.expect('PUNCTUATION', '(');
        const discriminant = this.parseExpression();
        this.expect('PUNCTUATION', ')');
        
        this.expect('PUNCTUATION', '{');
        const cases = [];
        
        while (this.getCurrentToken() && this.getCurrentToken().value !== '}') {
            const token = this.getCurrentToken();
            if (token.type === 'KEYWORD' && (token.value === 'case' || token.value === 'default')) {
                cases.push(this.parseSwitchCase());
            } else {
                // If there's something else inside switch that's not a case, 
                // it's technically invalid in strict C but we can skip or handle as error.
                // For now, let's skip it to avoid infinite loops if the user provides bad code.
                this.errors.push({
                    phase: 'Syntax',
                    message: `Expected 'case' or 'default' inside switch, found ${token.type} '${token.value}'`,
                    line: token.line,
                    column: token.column
                });
                this.consume();
            }
        }
        
        this.expect('PUNCTUATION', '}');
        
        return {
            type: 'SwitchStatement',
            discriminant: discriminant,
            cases: cases
        };
    }

    parseSwitchCase() {
        const token = this.consume(); // 'case' or 'default'
        let test = null;
        
        if (token.value === 'case') {
            test = this.parseExpression();
        }
        
        this.expect('PUNCTUATION', ':');
        
        const consequent = [];
        while (this.getCurrentToken() && 
               this.getCurrentToken().value !== '}' && 
               this.getCurrentToken().value !== 'case' && 
               this.getCurrentToken().value !== 'default') {
            const stmt = this.parseStatement();
            if (stmt) consequent.push(stmt);
        }
        
        return {
            type: 'SwitchCase',
            test: test,
            consequent: consequent
        };
    }

    parseReturn() {
        this.expect('KEYWORD', 'return');
        
        let value = null;
        if (this.getCurrentToken() && this.getCurrentToken().value !== ';') {
            value = this.parseExpression();
        }
        
        this.optionalSemicolon();
        
        return { type: 'ReturnStatement', value: value };
    }

    parseExpressionStatement() {
        const expr = this.parseExpression();
        this.optionalSemicolon();
        return { type: 'ExpressionStatement', expression: expr };
    }

    parseExpression() {
        return this.parseAssignment();
    }

    parseAssignment() {
        const left = this.parseConditional();
        
        if (this.getCurrentToken() && 
            ['=', '+=', '-=', '*=', '/=', '%='].includes(this.getCurrentToken().value)) {
            const operator = this.consume().value;
            const right = this.parseAssignment();
            return { type: 'AssignmentExpression', operator, left, right };
        }
        
        return left;
    }

    parseConditional() {
        const test = this.parseLogicalOr();
        
        if (this.getCurrentToken() && this.getCurrentToken().value === '?') {
            this.consume();
            const consequent = this.parseExpression();
            this.expect('PUNCTUATION', ':');
            const alternate = this.parseConditional();
            return { type: 'ConditionalExpression', test, consequent, alternate };
        }
        
        return test;
    }

    parseLogicalOr() {
        let left = this.parseLogicalAnd();
        
        while (this.getCurrentToken() && this.getCurrentToken().value === '||') {
            const operator = this.consume().value;
            const right = this.parseLogicalAnd();
            left = { type: 'LogicalExpression', operator, left, right };
        }
        
        return left;
    }

    parseLogicalAnd() {
        let left = this.parseEquality();
        
        while (this.getCurrentToken() && this.getCurrentToken().value === '&&') {
            const operator = this.consume().value;
            const right = this.parseEquality();
            left = { type: 'LogicalExpression', operator, left, right };
        }
        
        return left;
    }

    parseEquality() {
        let left = this.parseRelational();
        
        while (this.getCurrentToken() && ['==', '!='].includes(this.getCurrentToken().value)) {
            const operator = this.consume().value;
            const right = this.parseRelational();
            left = { type: 'BinaryExpression', operator, left, right };
        }
        
        return left;
    }

    parseRelational() {
        let left = this.parseAdditive();
        
        while (this.getCurrentToken() && ['<', '>', '<=', '>='].includes(this.getCurrentToken().value)) {
            const operator = this.consume().value;
            const right = this.parseAdditive();
            left = { type: 'BinaryExpression', operator, left, right };
        }
        
        return left;
    }

    parseAdditive() {
        let left = this.parseMultiplicative();
        
        while (this.getCurrentToken() && ['+', '-'].includes(this.getCurrentToken().value)) {
            const operator = this.consume().value;
            const right = this.parseMultiplicative();
            left = { type: 'BinaryExpression', operator, left, right };
        }
        
        return left;
    }

    parseMultiplicative() {
        let left = this.parseUnary();
        
        while (this.getCurrentToken() && ['*', '/', '%'].includes(this.getCurrentToken().value)) {
            const operator = this.consume().value;
            const right = this.parseUnary();
            left = { type: 'BinaryExpression', operator, left, right };
        }
        
        return left;
    }

    parseUnary() {
        const token = this.getCurrentToken();
        
        if (token && (['++', '--', '+', '-', '!', '~', '*', '&'].includes(token.value))) {
            const operator = this.consume().value;
            const argument = this.parseUnary();
            
            if (['++', '--'].includes(operator)) {
                return { type: 'UpdateExpression', operator, argument, prefix: true };
            }
            return { type: 'UnaryExpression', operator, argument };
        }
        
        return this.parsePostfix();
    }

    parsePostfix() {
        let expr = this.parsePrimary();
        
        while (this.getCurrentToken()) {
            const token = this.getCurrentToken();
            
            if (token.value === '(') {
                this.consume();
                const args = [];
                while (this.getCurrentToken() && this.getCurrentToken().value !== ')') {
                    args.push(this.parseExpression());
                    if (this.getCurrentToken().value === ',') this.consume();
                }
                this.expect('PUNCTUATION', ')');
                expr = { type: 'CallExpression', callee: expr, arguments: args };
            } else if (token.value === '[') {
                this.consume();
                const index = this.parseExpression();
                this.expect('PUNCTUATION', ']');
                expr = { type: 'MemberExpression', object: expr, property: index, computed: true };
            } else if (token.value === '.' || token.value === '->') {
                const operator = this.consume().value;
                const property = this.expect('IDENTIFIER').value;
                expr = { type: 'MemberExpression', object: expr, property: { type: 'Identifier', name: property }, computed: false, operator };
            } else if (['++', '--'].includes(token.value)) {
                const operator = this.consume().value;
                expr = { type: 'UpdateExpression', operator, argument: expr, prefix: false };
            } else {
                break;
            }
        }
        
        return expr;
    }

    parsePrimary() {
        const token = this.getCurrentToken();
        
        if (!token) throw new Error('Unexpected end of input');
        
        if (token.type === 'INTEGER') {
            this.consume();
            return { type: 'NumericLiteral', value: token.value };
        }
        if (token.type === 'FLOAT') {
            this.consume();
            return { type: 'NumericLiteral', value: token.value };
        }
        if (token.type === 'STRING') {
            this.consume();
            return { type: 'StringLiteral', value: token.value };
        }
        if (token.type === 'CHAR') {
            this.consume();
            return { type: 'CharLiteral', value: token.value };
        }
        if (token.type === 'IDENTIFIER') {
            this.consume();
            return { type: 'Identifier', name: token.value };
        }
        if (token.value === '(') {
            this.consume();
            const expr = this.parseExpression();
            this.expect('PUNCTUATION', ')');
            return expr;
        }
        
        throw new Error(`Unexpected token: ${token.type} '${token.value}' at line ${token.line}`);
    }

    expect(type, value) {
        const token = this.getCurrentToken();
        if (!token) {
            throw new Error(`Unexpected end of input, expected ${type}${value ? ` '${value}'` : ''}`);
        }
        if (token.type !== type || (value && token.value !== value)) {
            throw new Error(`Expected ${type}${value ? ` '${value}'` : ''}, but found ${token.type} '${token.value}' at line ${token.line}`);
        }
        return this.consume();
    }

    optionalSemicolon() {
        if (this.getCurrentToken() && this.getCurrentToken().value === ';') {
            this.consume();
        }
    }

    toArray(node) {
        if (!node) return [];
        if (node.type === 'BlockStatement') return node.body;
        if (Array.isArray(node)) return node;
        return [node];
    }

    formatAST(node, indent = '', isLast = true) {
        if (!node) return '';
        
        let result = '';
        const prefix = indent + (isLast ? '└── ' : '├── ');
        const childIndent = indent + (isLast ? '    ' : '│   ');
        
        switch (node.type) {
            case 'Program':
                result = `${indent}Program\n`;
                if (Array.isArray(node.body)) {
                    node.body.forEach((child, i) => {
                        result += this.formatAST(child, childIndent, i === node.body.length - 1);
                    });
                }
                break;
                
            case 'PreprocessorDirective':
                result = `${prefix}Preprocessor: ${node.value}\n`;
                break;
                
            case 'StructDeclaration':
                result = `${prefix}Struct: ${node.name || '<anonymous>'}\n`;
                if (node.members) {
                    node.members.forEach((member, i) => {
                        result += this.formatAST(member, childIndent, i === node.members.length - 1);
                    });
                }
                break;
                
            case 'StructMember':
                if (node.declarators) {
                    node.declarators.forEach((decl) => {
                        result += `${prefix}Member: ${decl.type} ${decl.name}\n`;
                    });
                }
                break;
                
            case 'FunctionDeclaration':
                result = `${prefix}Function: ${node.name}() → ${node.returnType}\n`;
                if (node.body) {
                    node.body.forEach((stmt, i) => {
                        result += this.formatAST(stmt, childIndent, i === node.body.length - 1);
                    });
                } else {
                    result += `${childIndent}(prototype)\n`;
                }
                break;
                
            case 'VariableDeclaration':
                result = `${prefix}Variable: ${node.kind} ${node.name}`;
                if (node.value) result += ` = ${this.formatValue(node.value)}`;
                result += '\n';
                break;
                
            case 'IfStatement':
                result = `${prefix}If (${this.formatValue(node.test)})\n`;
                if (node.consequent) {
                    node.consequent.forEach((stmt, i) => {
                        result += this.formatAST(stmt, childIndent, i === node.consequent.length - 1 && !node.alternate);
                    });
                }
                if (node.alternate) {
                    result += `${childIndent}Else:\n`;
                    node.alternate.forEach((stmt, i) => {
                        result += this.formatAST(stmt, childIndent, i === node.alternate.length - 1);
                    });
                }
                break;
                
            case 'WhileStatement':
                result = `${prefix}While (${this.formatValue(node.test)})\n`;
                if (node.body) {
                    node.body.forEach((stmt, i) => {
                        result += this.formatAST(stmt, childIndent, i === node.body.length - 1);
                    });
                }
                break;
                
            case 'ForStatement':
                result = `${prefix}For (${this.formatValue(node.init)}; ${this.formatValue(node.test)}; ${this.formatValue(node.update)})\n`;
                if (node.body) {
                    node.body.forEach((stmt, i) => {
                        result += this.formatAST(stmt, childIndent, i === node.body.length - 1);
                    });
                }
                break;
                
            case 'SwitchStatement':
                result = `${prefix}Switch (${this.formatValue(node.discriminant)})\n`;
                if (node.cases) {
                    node.cases.forEach((c, i) => {
                        result += this.formatAST(c, childIndent, i === node.cases.length - 1);
                    });
                }
                break;
                
            case 'SwitchCase':
                if (node.test) {
                    result = `${prefix}Case: ${this.formatValue(node.test)}\n`;
                } else {
                    result = `${prefix}Default:\n`;
                }
                if (node.consequent) {
                    node.consequent.forEach((stmt, i) => {
                        result += this.formatAST(stmt, childIndent, i === node.consequent.length - 1);
                    });
                }
                break;
                
            case 'ReturnStatement':
                result = `${prefix}Return`;
                if (node.value) result += ` ${this.formatValue(node.value)}`;
                result += '\n';
                break;
                
            case 'ExpressionStatement':
                result = `${prefix}Expr: ${this.formatValue(node.expression)}\n`;
                break;
                
            case 'CallExpression':
                result = `${prefix}Call: ${this.formatValue(node.callee)}()\n`;
                break;
                
            default:
                result = `${prefix}${node.type}\n`;
        }
        
        return result;
    }
    
    formatValue(node) {
        if (!node) return 'null';
        if (node.type === 'Identifier') return node.name;
        if (node.type === 'NumericLiteral' || node.type === 'StringLiteral') return node.value;
        if (node.type === 'BinaryExpression') {
            return `${this.formatValue(node.left)} ${node.operator} ${this.formatValue(node.right)}`;
        }
        if (node.type === 'CallExpression') {
            return `${this.formatValue(node.callee)}(${node.arguments.map(a => this.formatValue(a)).join(', ')})`;
        }
        return node.type;
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = Parser;
}