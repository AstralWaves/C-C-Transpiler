// Main Application Controller
/**
 * MAIN COMPILER APPLICATION
 * Responsibility: Orchestrates the entire compilation pipeline by connecting 
 * the Lexer, Parser, Semantic Analyzer, Optimizer, and Code Generator.
 * It also manages the UI updates and overall application state.
 */
class CompilerApp {
    constructor() {
        this.ui = new UIController();
        this.lexer = new Lexer();
        this.parser = new Parser();
        this.semantic = new SemanticAnalyzer();
        this.optimizer = new Optimizer();
        this.generator = new CodeGenerator();
        
        this.currentAST = null;
        this.currentTokens = null;
        this.init();
    }
    
    init() {
        this.ui.initializeEditors();
        this.ui.attachEventListeners();
        
        document.getElementById('compileBtn').addEventListener('click', () => this.compile());
        document.getElementById('clearBtn').addEventListener('click', () => this.clear());
        document.getElementById('clearConsole').addEventListener('click', () => this.ui.clearConsole());
        
        const langSelect = document.getElementById('targetLanguage');
        langSelect.addEventListener('change', (e) => {
            this.ui.updateOutputLanguage(e.target.value);
        });
        
        this.ui.updateOutputLanguage(langSelect.value);
        
        document.addEventListener('keydown', (e) => {
            if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
                e.preventDefault();
                this.compile();
            }
        });
        
        // Add sample code selector buttons
        this.addSampleButtons();
        
        this.ui.addConsoleMessage('✨ CompileX Ready! Try the sample code or write your own C/C++ code.', 'success');
    }
    
    addSampleButtons() {
        const header = document.querySelector('.header-controls');
        const sampleBtn = document.createElement('button');
        sampleBtn.className = 'clear-btn';
        sampleBtn.innerHTML = '<i class="fas fa-code"></i> Samples';
        sampleBtn.onclick = () => this.showSampleMenu();
        header.insertBefore(sampleBtn, document.getElementById('clearBtn'));
    }
    
    showSampleMenu() {
        const samples = {
            'Fibonacci': `int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n-1) + fibonacci(n-2);
}

int main() {
    int result = fibonacci(10);
    return result;
}`,
            'Factorial': `int factorial(int n) {
    int result = 1;
    for (int i = 1; i <= n; i++) {
        result = result * i;
    }
    return result;
}

int main() {
    int x = 5;
    int fact = factorial(x);
    return fact;
}`,
            'Simple Loop': `int main() {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum = sum + i;
    }
    return sum;
}`
        };
        
        // Simple prompt for sample selection
        const choice = prompt('Choose a sample:\n1. Fibonacci\n2. Factorial\n3. Simple Loop\n\nEnter number (1-3):');
        const keys = Object.keys(samples);
        const index = parseInt(choice) - 1;
        if (index >= 0 && index < keys.length) {
            this.ui.sourceEditor.setValue(samples[keys[index]]);
            this.ui.addConsoleMessage(`Loaded sample: ${keys[index]}`, 'info');
        }
    }
    
    async compile() {
        const startTime = performance.now();
        
        this.ui.clearPhases();
        this.ui.clearConsole();
        this.ui.setOutputCode('// Compiling...');
        this.ui.addConsoleMessage('🚀 Starting compilation process...', 'info');
        
        const sourceCode = this.ui.getSourceCode();
        
        if (!sourceCode.trim()) {
            this.ui.addConsoleMessage('❌ Error: No source code provided', 'error');
            this.ui.setOutputCode('// Error: No source code provided');
            return;
        }
        
        try {
            // Send to backend server for compilation
            const targetLang = document.getElementById('targetLanguage').value;
            
            this.ui.addConsoleMessage(`📡 Sending code to backend server...`, 'info');
            this.ui.updatePhaseStatus(1, 'Processing');
            
            const response = await fetch('/api/compile', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    code: sourceCode,
                    targetLanguage: targetLang
                })
            });
            
            const result = await response.json();
            
            if (!result.success) {
                throw new Error(result.error || 'Compilation failed');
            }
            
            // Update phases based on backend response
            this.ui.updatePhaseStatus(1, 'Completed', '✅ Lexical Analysis completed');
            this.ui.updatePhaseStatus(2, 'Completed', '✅ Syntax Analysis completed');
            this.ui.updatePhaseStatus(3, 'Completed', '✅ Semantic Analysis completed');
            this.ui.updatePhaseStatus(4, 'Completed', '✅ Intermediate Code generated');
            this.ui.updatePhaseStatus(5, 'Completed', '✅ Code Optimization completed');
            
            // Display the generated code
            this.ui.setOutputCode(result.target_code || result.output || '// No output generated');
            
            let codeStats = `Generated ${targetLang.toUpperCase()} Code\n`;
            codeStats += `━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n`;
            codeStats += `Lines: ${(result.target_code || result.output || '').split('\n').length}\n`;
            codeStats += `Characters: ${(result.target_code || result.output || '').length}\n`;
            codeStats += `Target: ${targetLang.charAt(0).toUpperCase() + targetLang.slice(1)}`;
            
            this.ui.updatePhaseStatus(6, 'Completed', codeStats);
            
            const endTime = performance.now();
            const compileTime = (endTime - startTime).toFixed(2);
            
            this.ui.showCompileTime(compileTime);
            this.ui.addConsoleMessage(`✅ Compilation completed successfully in ${compileTime}ms`, 'success');
            this.ui.addConsoleMessage(`📝 Output: ${targetLang.toUpperCase()} code generated`, 'success');
            
        } catch (error) {
            this.ui.addConsoleMessage(`❌ Compilation failed: ${error.message}`, 'error');
            this.ui.setOutputCode(`// Compilation failed:\n// ${error.message}`);
            console.error('Compilation error:', error);
        }
    }
    
    async phase1(sourceCode) {
        this.ui.updatePhaseStatus(1, 'Processing');
        this.ui.addConsoleMessage('📝 Phase 1: Lexical Analysis - Tokenizing source code...', 'info');
        
        await this.delay(150);
        
        const result = this.lexer.tokenize(sourceCode);
        this.currentTokens = result.tokens;
        
        if (result.errors.length > 0) {
            this.ui.updatePhaseStatus(1, 'Error', result.errors[0].message);
            result.errors.forEach(err => {
                this.ui.addConsoleMessage(`❌ Lexical Error: ${err.message}`, 'error');
            });
            throw new Error('Lexical analysis failed');
        }
        
        const summary = this.lexer.getTokenSummary();
        const tokenOutput = this.lexer.formatTokens();
        
        // Create beautiful token visualization
        let tokenDisplay = `📊 Total Tokens: ${result.totalTokens}\n`;
        tokenDisplay += `━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n`;
        tokenDisplay += `Token Distribution:\n`;
        Object.entries(summary).sort((a, b) => b[1] - a[1]).forEach(([type, count]) => {
            const bar = '█'.repeat(Math.min(20, count));
            tokenDisplay += `  ${type.padEnd(15)}: ${bar} ${count}\n`;
        });
        tokenDisplay += `\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n`;
        tokenDisplay += tokenOutput;
        
        this.ui.updatePhaseStatus(1, 'Completed', tokenDisplay);
        this.ui.addConsoleMessage(`✅ Lexical Analysis: Generated ${result.totalTokens} tokens`, 'success');
        
        return result;
    }
    
    async phase2() {
        this.ui.updatePhaseStatus(2, 'Processing');
        this.ui.addConsoleMessage('🌳 Phase 2: Syntax Analysis - Building Abstract Syntax Tree...', 'info');
        
        await this.delay(150);
        
        const result = this.parser.parse(this.currentTokens);
        
        if (!result.success) {
            const errorMsg = result.errors.length > 0 ? result.errors[0].message : 'Unknown syntax error';
            this.ui.updatePhaseStatus(2, 'Error', errorMsg);
            result.errors.forEach(err => {
                this.ui.addConsoleMessage(`❌ Syntax Error: ${err.message} at line ${err.line}`, 'error');
            });
            throw new Error('Syntax analysis failed');
        }
        
        this.currentAST = result.ast;
        
        // Create beautiful AST visualization
        let astDisplay = `🌲 Abstract Syntax Tree\n`;
        astDisplay += `━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n`;
        astDisplay += this.parser.formatAST(this.currentAST);
        
        this.ui.updatePhaseStatus(2, 'Completed', astDisplay);
        this.ui.addConsoleMessage('✅ Syntax Analysis: AST built successfully', 'success');
        
        return result;
    }
    
    async phase3() {
        this.ui.updatePhaseStatus(3, 'Processing');
        this.ui.addConsoleMessage('🔍 Phase 3: Semantic Analysis - Type checking and symbol resolution...', 'info');
        
        await this.delay(150);
        
        const result = this.semantic.analyze(this.currentAST);
        
        if (!result.success) {
            const errorMsg = result.errors.length > 0 ? result.errors[0].message : 'Unknown semantic error';
            this.ui.updatePhaseStatus(3, 'Error', errorMsg);
            result.errors.forEach(err => {
                this.ui.addConsoleMessage(`❌ Semantic Error: ${err.message}`, 'error');
            });
            throw new Error('Semantic analysis failed');
        }
        
        if (result.warnings.length > 0) {
            result.warnings.forEach(warn => {
                this.ui.addConsoleMessage(`⚠️ Semantic Warning: ${warn.message}`, 'warning');
            });
        }
        
        const symbolTableOutput = this.semantic.formatSymbolTable();
        
        this.ui.updatePhaseStatus(3, 'Completed', symbolTableOutput);
        this.ui.addConsoleMessage(`✅ Semantic Analysis: ${this.semantic.symbolTable.size} symbols analyzed`, 'success');
        
        return result;
    }
    
    async phase4() {
        this.ui.updatePhaseStatus(4, 'Processing');
        this.ui.addConsoleMessage('🔄 Phase 4: Intermediate Code Generation...', 'info');
        
        await this.delay(150);
        
        const ir = this.generateIR(this.currentAST);
        
        let irDisplay = `📋 Three-Address Code (Intermediate Representation)\n`;
        irDisplay += `━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n`;
        irDisplay += ir;
        
        this.ui.updatePhaseStatus(4, 'Completed', irDisplay);
        this.ui.addConsoleMessage('✅ Intermediate Code: Generated successfully', 'success');
        
        return ir;
    }
    
    async phase5() {
        this.ui.updatePhaseStatus(5, 'Processing');
        this.ui.addConsoleMessage('⚡ Phase 5: Code Optimization...', 'info');
        
        await this.delay(150);
        
        const result = this.optimizer.optimize(this.currentAST);
        this.currentAST = result.ast;
        
        const optimizationOutput = this.optimizer.getOptimizationSummary();
        
        this.ui.updatePhaseStatus(5, 'Completed', optimizationOutput);
        this.ui.addConsoleMessage(`✅ Optimization: Applied ${result.optimizations.length} optimizations`, 'success');
        
        return result;
    }
    
    async phase6() {
        this.ui.updatePhaseStatus(6, 'Processing');
        this.ui.addConsoleMessage('🎯 Phase 6: Code Generation...', 'info');
        
        await this.delay(150);
        
        const targetLang = document.getElementById('targetLanguage').value;
        const generatedCode = this.generator.generate(this.currentAST, targetLang);
        
        this.ui.setOutputCode(generatedCode);
        
        let codeStats = `Generated ${targetLang.toUpperCase()} Code\n`;
        codeStats += `━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n`;
        codeStats += `Lines: ${generatedCode.split('\n').length}\n`;
        codeStats += `Characters: ${generatedCode.length}\n`;
        codeStats += `Target: ${targetLang.charAt(0).toUpperCase() + targetLang.slice(1)}`;
        
        this.ui.updatePhaseStatus(6, 'Completed', codeStats);
        this.ui.addConsoleMessage(`✅ Code Generation: ${targetLang.toUpperCase()} code generated (${generatedCode.split('\n').length} lines)`, 'success');
        
        return generatedCode;
    }
    
    generateIR(ast) {
        const ir = [];
        let tempCount = 1;
        let labelCount = 1;
        
        const visit = (node) => {
            if (!node) return null;
            
            switch (node.type) {
                case 'Program':
                    if (Array.isArray(node.body)) {
                        node.body.forEach(stmt => visit(stmt));
                    }
                    return null;
                    
                case 'FunctionDeclaration':
                    ir.push(`\n┌─ Function: ${node.name}`);
                    ir.push(`│  Parameters: ${node.params.map(p => p.name).join(', ')}`);
                    if (Array.isArray(node.body)) {
                        node.body.forEach(stmt => visit(stmt));
                    }
                    ir.push(`└─ End Function ${node.name}\n`);
                    return null;

                case 'BinaryExpression':
                    const left = visit(node.left);
                    const right = visit(node.right);
                    const temp = `t${tempCount++}`;
                    ir.push(`│  ${temp} = ${left} ${node.operator} ${right}`);
                    return temp;
                    
                case 'NumericLiteral':
                    return node.value;
                    
                case 'Identifier':
                    return node.name;
                    
                case 'AssignmentExpression':
                    const rightVal = visit(node.right);
                    const leftName = visit(node.left);
                    ir.push(`│  ${leftName} = ${rightVal}`);
                    return leftName;

                case 'VariableDeclaration':
                    if (node.value) {
                        const val = visit(node.value);
                        ir.push(`│  ${node.name} = ${val}`);
                    } else {
                        ir.push(`│  declare ${node.name}`);
                    }
                    return node.name;

                case 'ReturnStatement':
                    if (node.value) {
                        const retVal = visit(node.value);
                        ir.push(`│  return ${retVal}`);
                    } else {
                        ir.push(`│  return`);
                    }
                    return null;

                case 'IfStatement':
                    const cond = visit(node.test);
                    const labelTrue = `L${labelCount++}`;
                    const labelEnd = `L${labelCount++}`;
                    ir.push(`│  if ${cond} goto ${labelTrue}`);
                    
                    if (node.alternate) {
                        const labelFalse = `L${labelCount++}`;
                        ir.push(`│  goto ${labelFalse}`);
                        ir.push(`├─ ${labelTrue}:`);
                        if (Array.isArray(node.consequent)) {
                            node.consequent.forEach(s => visit(s));
                        }
                        ir.push(`│  goto ${labelEnd}`);
                        ir.push(`├─ ${labelFalse}:`);
                        if (Array.isArray(node.alternate)) {
                            node.alternate.forEach(s => visit(s));
                        }
                    } else {
                        ir.push(`│  goto ${labelEnd}`);
                        ir.push(`├─ ${labelTrue}:`);
                        if (Array.isArray(node.consequent)) {
                            node.consequent.forEach(s => visit(s));
                        }
                    }
                    ir.push(`└─ ${labelEnd}:`);
                    return null;

                case 'ForStatement':
                    if (node.init) visit(node.init);
                    const forStart = `L${labelCount++}`;
                    const forEnd = `L${labelCount++}`;
                    ir.push(`├─ ${forStart}:`);
                    if (node.test) {
                        const forCond = visit(node.test);
                        ir.push(`│  ifnot ${forCond} goto ${forEnd}`);
                    }
                    if (Array.isArray(node.body)) {
                        node.body.forEach(s => visit(s));
                    }
                    if (node.update) visit(node.update);
                    ir.push(`│  goto ${forStart}`);
                    ir.push(`└─ ${forEnd}:`);
                    return null;

                case 'WhileStatement':
                    const whileStart = `L${labelCount++}`;
                    const whileEnd = `L${labelCount++}`;
                    ir.push(`├─ ${whileStart}:`);
                    const whileCond = visit(node.test);
                    ir.push(`│  ifnot ${whileCond} goto ${whileEnd}`);
                    if (Array.isArray(node.body)) {
                        node.body.forEach(s => visit(s));
                    }
                    ir.push(`│  goto ${whileStart}`);
                    ir.push(`└─ ${whileEnd}:`);
                    return null;

                case 'SwitchStatement':
                    const switchVal = visit(node.discriminant);
                    const switchEnd = `L${labelCount++}`;
                    ir.push(`│  switch ${switchVal}`);
                    if (Array.isArray(node.cases)) {
                        node.cases.forEach(c => {
                            if (c.test) {
                                const caseVal = visit(c.test);
                                const caseLabel = `L${labelCount++}`;
                                ir.push(`│  if ${switchVal} == ${caseVal} goto ${caseLabel}`);
                                // We'll handle the actual jumping in a simplified way for IR
                            }
                        });
                        ir.push(`│  goto ${switchEnd}`); // Default or end
                        node.cases.forEach(c => {
                            if (c.test) {
                                // Re-using labels for simplicity in visualization
                                ir.push(`├─ case ${this.parser.formatValue(c.test)}:`);
                            } else {
                                ir.push(`├─ default:`);
                            }
                            if (Array.isArray(c.consequent)) {
                                c.consequent.forEach(s => visit(s));
                            }
                        });
                    }
                    ir.push(`└─ ${switchEnd}:`);
                    return null;

                case 'CallExpression':
                    const args = node.arguments.map(a => visit(a)).join(', ');
                    const tempCall = `t${tempCount++}`;
                    ir.push(`│  ${tempCall} = call ${node.callee}(${args})`);
                    return tempCall;
            }
            return null;
        };
        
        visit(ast);
        
        return ir.length > 0 ? ir.join('\n') : 'No intermediate code generated';
    }
    
    delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
    
    clear() {
        this.ui.sourceEditor.setValue('');
        this.ui.outputEditor.setValue('// Generated code will appear here...');
        this.ui.clearPhases();
        this.ui.clearConsole();
        this.currentAST = null;
        this.currentTokens = null;
        this.ui.addConsoleMessage('🧹 Editor cleared. Ready for new input.', 'info');
    }
}

document.addEventListener('DOMContentLoaded', () => {
    window.app = new CompilerApp();
}); 