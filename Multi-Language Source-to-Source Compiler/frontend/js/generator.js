// Code Generation Phase
class CodeGenerator {
    constructor() {
        this.targetLanguage = 'java';
        this.indentLevel = 0;
        this.output = '';
    }
    
    generate(ast, targetLanguage) {
        this.targetLanguage = targetLanguage;
        this.indentLevel = 0;
        this.output = '';
        
        this.generateNode(ast);
        
        return this.output;
    }
    
    indent() {
        return '    '.repeat(this.indentLevel);
    }
    
    emit(code) {
        this.output += code;
    }
    
    emitLine(code = '') {
        this.output += this.indent() + code + '\n';
    }
    
    generateNode(node) {
        if (!node) return;
        
        switch (node.type) {
            case 'Program':
                this.generateProgram(node);
                break;
            case 'FunctionDeclaration':
                this.generateFunctionDeclaration(node);
                break;
            case 'BlockStatement':
                this.generateBlockStatement(node);
                break;
            case 'VariableDeclaration':
                this.generateVariableDeclaration(node);
                break;
            case 'IfStatement':
                this.generateIfStatement(node);
                break;
            case 'WhileStatement':
                this.generateWhileStatement(node);
                break;
            case 'ForStatement':
                this.generateForStatement(node);
                break;
            case 'SwitchStatement':
                this.generateSwitchStatement(node);
                break;
            case 'SwitchCase':
                this.generateSwitchCase(node);
                break;
            case 'ReturnStatement':
                this.generateReturnStatement(node);
                break;
            case 'BreakStatement':
                this.generateBreakStatement(node);
                break;
            case 'ContinueStatement':
                this.generateContinueStatement(node);
                break;
            case 'ExpressionStatement':
                this.generateExpressionStatement(node);
                break;
            case 'BinaryExpression':
                this.generateBinaryExpression(node);
                break;
            case 'AssignmentExpression':
                this.generateAssignmentExpression(node);
                break;
            case 'UpdateExpression':
                this.generateUpdateExpression(node);
                break;
            case 'CallExpression':
                this.generateCallExpression(node);
                break;
            case 'MemberExpression':
                this.generateMemberExpression(node);
                break;
            case 'Identifier':
                this.generateIdentifier(node);
                break;
            case 'NumericLiteral':
                this.generateNumericLiteral(node);
                break;
            case 'StringLiteral':
                this.generateStringLiteral(node);
                break;
            case 'PreprocessorDirective':
                this.generatePreprocessorDirective(node);
                break;
            case 'ErrorStatement':
                this.emitLine(`// Error: ${node.message}`);
                break;
        }
    }
    
    generatePreprocessorDirective(node) {
        if (this.targetLanguage === 'javascript') {
            this.emitLine(`// ${node.value}`);
        } else {
            this.emitLine(node.value);
        }
    }
    
    generateProgram(node) {
        if (this.targetLanguage === 'java') {
            this.emitLine('public class Program {');
            this.indentLevel++;
            this.emitLine('public static void main(String[] args) {');
            this.indentLevel++;
        } else if (this.targetLanguage === 'csharp') {
            this.emitLine('using System;');
            this.emitLine();
            this.emitLine('class Program {');
            this.indentLevel++;
            this.emitLine('static void Main(string[] args) {');
            this.indentLevel++;
        } else if (this.targetLanguage === 'javascript') {
            this.emitLine('// Generated JavaScript Code');
            this.emitLine();
        }
        
        if (Array.isArray(node.body)) {
            node.body.forEach(stmt => this.generateNode(stmt));
        }
        
        if (this.targetLanguage === 'java' || this.targetLanguage === 'csharp') {
            this.indentLevel--;
            this.emitLine('}');
            this.indentLevel--;
            this.emitLine('}');
        }
    }
    
    generateFunctionDeclaration(node) {
        let returnType = this.mapType(node.returnType);
        
        if (this.targetLanguage === 'java') {
            this.emit(`public static ${returnType} ${node.name}(`);
        } else if (this.targetLanguage === 'csharp') {
            this.emit(`static ${returnType} ${node.name}(`);
        } else if (this.targetLanguage === 'javascript') {
            this.emit(`function ${node.name}(`);
        }
        
        if (Array.isArray(node.params)) {
            node.params.forEach((param, index) => {
                if (index > 0) this.emit(', ');
                
                if (this.targetLanguage !== 'javascript') {
                    this.emit(`${this.mapType(param.type)} ${param.name}`);
                } else {
                    this.emit(param.name);
                }
            });
        }
        
        this.emitLine(') {');
        this.indentLevel++;
        
        if (Array.isArray(node.body)) {
            node.body.forEach(stmt => this.generateNode(stmt));
        } else {
            this.generateNode(node.body);
        }
        
        this.indentLevel--;
        this.emitLine('}');
    }
    
    generateBlockStatement(node) {
        if (this.indentLevel === 0 && this.targetLanguage !== 'javascript') {
            this.emitLine('{');
            this.indentLevel++;
        }
        
        if (Array.isArray(node.body)) {
            node.body.forEach(stmt => this.generateNode(stmt));
        }
        
        if (this.indentLevel === 0 && this.targetLanguage !== 'javascript') {
            this.indentLevel--;
            this.emitLine('}');
        }
    }
    
    generateVariableDeclaration(node) {
        const type = this.mapType(node.kind);
        
        this.emit(this.indent());
        
        if (this.targetLanguage !== 'javascript') {
            this.emit(`${type} ${node.name}`);
        } else {
            this.emit(`let ${node.name}`);
        }
        
        if (node.value) {
            this.emit(' = ');
            this.generateNode(node.value);
        }
        
        this.emitLine(';');
    }
    
    generateIfStatement(node) {
        this.emit(this.indent() + 'if (');
        this.generateNode(node.test);
        this.emitLine(') {');
        
        this.indentLevel++;
        if (Array.isArray(node.consequent)) {
            node.consequent.forEach(stmt => this.generateNode(stmt));
        } else {
            this.generateNode(node.consequent);
        }
        this.indentLevel--;
        
        this.emitLine(this.indent() + '}');
        
        if (node.alternate) {
            this.emit(this.indent() + 'else ');
            
            if (node.alternate.type === 'IfStatement') {
                this.generateNode(node.alternate);
            } else {
                this.emitLine('{');
                this.indentLevel++;
                if (Array.isArray(node.alternate)) {
                    node.alternate.forEach(stmt => this.generateNode(stmt));
                } else {
                    this.generateNode(node.alternate);
                }
                this.indentLevel--;
                this.emitLine(this.indent() + '}');
            }
        }
    }
    
    generateWhileStatement(node) {
        this.emit(this.indent() + 'while (');
        this.generateNode(node.test);
        this.emitLine(') {');
        
        this.indentLevel++;
        if (Array.isArray(node.body)) {
            node.body.forEach(stmt => this.generateNode(stmt));
        } else {
            this.generateNode(node.body);
        }
        this.indentLevel--;
        
        this.emitLine(this.indent() + '}');
    }
    
    generateForStatement(node) {
        this.emit(this.indent() + 'for (');
        
        if (node.init) {
            this.generateNode(node.init);
            // Remove trailing semicolon for C-style for loops
            this.output = this.output.slice(0, -1);
        } else {
            this.emit('; ');
        }
        
        if (node.test) {
            this.generateNode(node.test);
        }
        this.emit('; ');
        
        if (node.update) {
            const updateStr = this.generateToString(node.update);
            this.emit(updateStr);
        }
        
        this.emitLine(') {');
        
        this.indentLevel++;
        if (Array.isArray(node.body)) {
            node.body.forEach(stmt => this.generateNode(stmt));
        } else {
            this.generateNode(node.body);
        }
        this.indentLevel--;
        
        this.emitLine(this.indent() + '}');
    }
    
    generateSwitchStatement(node) {
        this.emit(this.indent() + 'switch (');
        this.generateNode(node.discriminant);
        this.emitLine(') {');
        
        this.indentLevel++;
        if (Array.isArray(node.cases)) {
            node.cases.forEach(c => this.generateNode(c));
        }
        this.indentLevel--;
        
        this.emitLine(this.indent() + '}');
    }
    
    generateSwitchCase(node) {
        if (node.test) {
            this.emit(this.indent() + 'case ');
            this.generateNode(node.test);
            this.emitLine(':');
        } else {
            this.emitLine('default:');
        }
        
        this.indentLevel++;
        if (Array.isArray(node.consequent)) {
            node.consequent.forEach(stmt => this.generateNode(stmt));
        }
        this.indentLevel--;
    }
    
    generateReturnStatement(node) {
        this.emit(this.indent() + 'return');
        
        if (node.value) {
            this.emit(' ');
            this.generateNode(node.value);
        }
        
        this.emitLine(';');
    }

    generateBreakStatement(node) {
        this.emitLine('break;');
    }

    generateContinueStatement(node) {
        this.emitLine('continue;');
    }
    
    generateExpressionStatement(node) {
        this.emit(this.indent());
        this.generateNode(node.expression);
        this.emitLine(';');
    }
    
    generateBinaryExpression(node) {
        this.generateNode(node.left);
        this.emit(` ${node.operator} `);
        this.generateNode(node.right);
    }
    
    generateAssignmentExpression(node) {
        this.generateNode(node.left);
        this.emit(` ${node.operator} `);
        this.generateNode(node.right);
    }
    
    generateUpdateExpression(node) {
        if (node.prefix) {
            this.emit(node.operator);
            this.generateNode(node.argument);
        } else {
            this.generateNode(node.argument);
            this.emit(node.operator);
        }
    }
    
    generateCallExpression(node) {
        if (typeof node.callee === 'string') {
            this.emit(node.callee);
        } else {
            this.generateNode(node.callee);
        }
        this.emit('(');
        
        if (Array.isArray(node.arguments)) {
            node.arguments.forEach((arg, index) => {
                if (index > 0) this.emit(', ');
                this.generateNode(arg);
            });
        }
        
        this.emit(')');
    }

    generateMemberExpression(node) {
        this.generateNode(node.object);
        if (node.computed) {
            this.emit('[');
            this.generateNode(node.property);
            this.emit(']');
        } else {
            const op = this.targetLanguage === 'javascript' ? '.' : (node.operator || '.');
            this.emit(op);
            this.generateNode(node.property);
        }
    }
    
    generateIdentifier(node) {
        this.emit(node.name);
    }
    
    generateNumericLiteral(node) {
        this.emit(node.value);
    }

    generateStringLiteral(node) {
        this.emit(`"${node.value}"`);
    }
    
    generateToString(node) {
        const oldOutput = this.output;
        this.output = '';
        this.generateNode(node);
        const result = this.output;
        this.output = oldOutput;
        return result;
    }
    
    mapType(cType) {
        const typeMap = {
            'int': 'int',
            'float': 'float',
            'double': 'double',
            'char': 'char',
            'void': 'void'
        };
        
        if (this.targetLanguage === 'java') {
            typeMap['float'] = 'float';
            typeMap['double'] = 'double';
        } else if (this.targetLanguage === 'csharp') {
            typeMap['float'] = 'float';
            typeMap['double'] = 'double';
        } else if (this.targetLanguage === 'javascript') {
            return ''; // JavaScript doesn't use type declarations
        }
        
        return typeMap[cType] || cType;
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = CodeGenerator;
}