// Lexical Analysis Phase - Tokenization
/**
 * LEXICAL ANALYSIS PHASE (SCANNER)
 * Responsibility: Converts raw source code characters into a list of meaningful 'Tokens'.
 * It identifies keywords, identifiers, numbers, and operators while ignoring whitespace.
 */
class Lexer {
    constructor() {
        this.tokens = [];
        this.errors = [];
        // Standard C and universal keywords
        this.keywords = new Set([
            'int', 'float', 'double', 'char', 'void', 'return',
            'if', 'else', 'for', 'while', 'do', 'break', 'continue',
            'switch', 'case', 'default', 'struct', 'class', 'public',
            'private', 'protected', 'static', 'const', 'using', 'namespace',
            'include', 'define', 'ifdef', 'endif', 'let', 'var', 'function',
            'string', 'bool', 'auto', 'def', 'print', 'println', 'cout', 'cin'
        ]);
        
        this.operators = new Set([
            '+', '-', '*', '/', '%', '=', '==', '!=', '<', '>', '<=', '>=',
            '&&', '||', '!', '&', '|', '^', '~', '<<', '>>', '++', '--',
            '+=', '-=', '*=', '/=', '%=', '&=', '|=', '^='
        ]);
        
        this.delimiters = new Set([
            '(', ')', '{', '}', '[', ']', ';', ',', '.', ':', '::', '->'
        ]);
    }
    
    tokenize(sourceCode) {
        this.tokens = [];
        this.errors = [];
        
        let position = 0;
        let line = 1;
        let column = 1;
        
        while (position < sourceCode.length) {
            let char = sourceCode[position];
            
            // Skip whitespace
            if (/\s/.test(char)) {
                if (char === '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                position++;
                continue;
            }
            
            // Skip comments
            if (char === '/' && position + 1 < sourceCode.length) {
                if (sourceCode[position + 1] === '/') {
                    position += 2;
                    column += 2;
                    while (position < sourceCode.length && sourceCode[position] !== '\n') {
                        position++;
                        column++;
                    }
                    continue;
                } else if (sourceCode[position + 1] === '*') {
                    position += 2;
                    column += 2;
                    while (position + 1 < sourceCode.length && 
                           !(sourceCode[position] === '*' && sourceCode[position + 1] === '/')) {
                        if (sourceCode[position] === '\n') {
                            line++;
                            column = 1;
                        } else {
                            column++;
                        }
                        position++;
                    }
                    position += 2;
                    column += 2;
                    continue;
                }
            }
            
            // Preprocessor directives
            if (char === '#') {
                let directive = '';
                let startColumn = column;
                
                while (position < sourceCode.length && sourceCode[position] !== '\n') {
                    directive += sourceCode[position];
                    position++;
                    column++;
                }
                
                this.tokens.push({
                    type: 'PREPROCESSOR',
                    value: directive,
                    line: line,
                    column: startColumn
                });
                continue;
            }
            
            // Identifiers and keywords
            if (/[a-zA-Z_]/.test(char)) {
                let identifier = '';
                let startColumn = column;
                
                while (position < sourceCode.length && /[a-zA-Z0-9_]/.test(sourceCode[position])) {
                    identifier += sourceCode[position];
                    position++;
                    column++;
                }
                
                const type = this.keywords.has(identifier) ? 'KEYWORD' : 'IDENTIFIER';
                this.tokens.push({
                    type: type,
                    value: identifier,
                    line: line,
                    column: startColumn
                });
                continue;
            }
            
            // Numbers - FIXED: Separate INTEGER and FLOAT tokens
            if (/[0-9]/.test(char)) {
                let number = '';
                let startColumn = column;
                let isFloat = false;
                
                while (position < sourceCode.length && /[0-9.]/.test(sourceCode[position])) {
                    if (sourceCode[position] === '.') {
                        if (isFloat) {
                            this.errors.push({
                                phase: 'Lexical',
                                message: `Invalid number format at line ${line}, column ${column}`,
                                line: line,
                                column: column
                            });
                            break;
                        }
                        isFloat = true;
                    }
                    number += sourceCode[position];
                    position++;
                    column++;
                }
                
                // FIXED: Use INTEGER or FLOAT based on presence of decimal
                this.tokens.push({
                    type: isFloat ? 'FLOAT' : 'INTEGER',
                    value: number,
                    line: line,
                    column: startColumn
                });
                continue;
            }
            
            // String literals
            if (char === '"') {
                let string = '';
                let startColumn = column;
                position++;
                column++;
                
                while (position < sourceCode.length && sourceCode[position] !== '"') {
                    if (sourceCode[position] === '\n') {
                        this.errors.push({
                            phase: 'Lexical',
                            message: `Unterminated string literal at line ${line}`,
                            line: line,
                            column: column
                        });
                        break;
                    }
                    
                    if (sourceCode[position] === '\\') {
                        string += sourceCode[position];
                        position++;
                        column++;
                        if (position < sourceCode.length) {
                            string += sourceCode[position];
                            position++;
                            column++;
                        }
                    } else {
                        string += sourceCode[position];
                        position++;
                        column++;
                    }
                }
                
                if (position < sourceCode.length && sourceCode[position] === '"') {
                    position++;
                    column++;
                }
                
                this.tokens.push({
                    type: 'STRING',
                    value: string,
                    line: line,
                    column: startColumn
                });
                continue;
            }
            
            // Operators and delimiters
            let found = false;
            
            // Check for two-character operators
            if (position + 1 < sourceCode.length) {
                const twoChar = char + sourceCode[position + 1];
                if (this.operators.has(twoChar) || this.delimiters.has(twoChar)) {
                    this.tokens.push({
                        type: this.operators.has(twoChar) ? 'OPERATOR' : 'PUNCTUATION',
                        value: twoChar,
                        line: line,
                        column: column
                    });
                    position += 2;
                    column += 2;
                    found = true;
                }
            }
            
            if (!found) {
                if (this.operators.has(char) || this.delimiters.has(char)) {
                    this.tokens.push({
                        type: this.operators.has(char) ? 'OPERATOR' : 'PUNCTUATION',
                        value: char,
                        line: line,
                        column: column
                    });
                    position++;
                    column++;
                    found = true;
                }
            }
            
            if (!found) {
                this.errors.push({
                    phase: 'Lexical',
                    message: `Unknown character '${char}' at line ${line}, column ${column}`,
                    line: line,
                    column: column
                });
                position++;
                column++;
            }
        }
        
        this.tokens.push({
            type: 'EOF',
            value: 'EOF',
            line: line,
            column: column
        });
        
        return {
            tokens: this.tokens,
            errors: this.errors,
            totalTokens: this.tokens.length
        };
    }
    
    getTokenSummary() {
        const summary = {};
        this.tokens.forEach(token => {
            if (token.type !== 'EOF') {
                summary[token.type] = (summary[token.type] || 0) + 1;
            }
        });
        return summary;
    }
    
    formatTokens() {
        const lines = [];
        this.tokens.forEach((token, index) => {
            if (token.type !== 'EOF' && index < 20) { // Show first 20 tokens
                lines.push(`[${index}] ${token.type}: '${token.value}' at line ${token.line}:${token.column}`);
            }
        });
        if (this.tokens.length > 21) {
            lines.push(`... and ${this.tokens.length - 21} more tokens`);
        }
        return lines.join('\n');
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = Lexer;
}