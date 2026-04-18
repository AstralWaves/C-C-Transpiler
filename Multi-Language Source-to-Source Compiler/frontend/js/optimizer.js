// Code Optimization Phase
class Optimizer {
    constructor() {
        this.optimizations = [];
    }
    
    optimize(ast) {
        this.optimizations = [];
        
        // Perform various optimizations
        let optimizedAST = this.cloneAST(ast);
        
        optimizedAST = this.constantFolding(optimizedAST);
        optimizedAST = this.deadCodeElimination(optimizedAST);
        optimizedAST = this.strengthReduction(optimizedAST);
        
        return {
            ast: optimizedAST,
            optimizations: this.optimizations
        };
    }
    
    cloneAST(node) {
        if (!node) return null;
        return JSON.parse(JSON.stringify(node));
    }
    
    constantFolding(node) {
        const visitor = (node) => {
            if (!node) return node;
            
            if (node.type === 'BinaryExpression') {
                node.left = visitor(node.left);
                node.right = visitor(node.right);
                
                // Check if both operands are numeric literals
                if (node.left?.type === 'NumericLiteral' && node.right?.type === 'NumericLiteral') {
                    const leftVal = parseFloat(node.left.value);
                    const rightVal = parseFloat(node.right.value);
                    let result;
                    
                    try {
                        switch (node.operator) {
                            case '+': result = leftVal + rightVal; break;
                            case '-': result = leftVal - rightVal; break;
                            case '*': result = leftVal * rightVal; break;
                            case '/': 
                                if (rightVal === 0) return node;
                                result = leftVal / rightVal; 
                                break;
                            case '%': result = leftVal % rightVal; break;
                            case '<': result = leftVal < rightVal ? 1 : 0; break;
                            case '>': result = leftVal > rightVal ? 1 : 0; break;
                            case '<=': result = leftVal <= rightVal ? 1 : 0; break;
                            case '>=': result = leftVal >= rightVal ? 1 : 0; break;
                            case '==': result = leftVal === rightVal ? 1 : 0; break;
                            case '!=': result = leftVal !== rightVal ? 1 : 0; break;
                            default: return node;
                        }
                        
                        this.optimizations.push({
                            type: 'Constant Folding',
                            description: `Folded ${leftVal} ${node.operator} ${rightVal} = ${result}`
                        });
                        
                        return {
                            type: 'NumericLiteral',
                            value: String(result)
                        };
                    } catch (e) {
                        // Operation failed, return original node
                        return node;
                    }
                }
            } else if (node.type === 'IfStatement') {
                node.test = visitor(node.test);
                
                // If condition is constant, eliminate dead branch
                if (node.test?.type === 'NumericLiteral') {
                    const condition = parseFloat(node.test.value);
                    
                    this.optimizations.push({
                        type: 'Dead Code Elimination',
                        description: `Eliminated ${condition ? 'else' : 'then'} branch of if statement`
                    });
                    
                    // Return the consequent as a block if it's not already handled
                    return {
                        type: 'BlockStatement',
                        body: condition ? node.consequent : (node.alternate || [])
                    };
                }
                
                if (Array.isArray(node.consequent)) {
                    node.consequent = node.consequent.map(stmt => visitor(stmt)).filter(stmt => stmt !== null);
                } else {
                    node.consequent = visitor(node.consequent);
                }

                if (node.alternate) {
                    if (Array.isArray(node.alternate)) {
                        node.alternate = node.alternate.map(stmt => visitor(stmt)).filter(stmt => stmt !== null);
                    } else {
                        node.alternate = visitor(node.alternate);
                    }
                }
            } else if (node.type === 'WhileStatement') {
                node.test = visitor(node.test);
                if (Array.isArray(node.body)) {
                    node.body = node.body.map(stmt => visitor(stmt)).filter(stmt => stmt !== null);
                } else {
                    node.body = visitor(node.body);
                }
                
                // If condition is false, eliminate loop
                if (node.test?.type === 'NumericLiteral' && parseFloat(node.test.value) === 0) {
                    this.optimizations.push({
                        type: 'Dead Code Elimination',
                        description: 'Eliminated while loop with false condition'
                    });
                    return null;
                }
            } else if (node.body && Array.isArray(node.body)) {
                node.body = node.body.map(stmt => visitor(stmt)).filter(stmt => stmt !== null);
            } else if (node.type === 'BinaryExpression') {
                node.left = visitor(node.left);
                node.right = visitor(node.right);
            } else if (node.type === 'UpdateExpression') {
                node.argument = visitor(node.argument);
            } else if (node.type === 'MemberExpression') {
                node.object = visitor(node.object);
                if (node.computed) node.property = visitor(node.property);
            } else if (node.body) {
                node.body = visitor(node.body);
            }
            
            return node;
        };
        
        return visitor(node);
    }
    
    deadCodeElimination(node) {
        const visitor = (node) => {
            if (!node) return node;
            
            if (node.type === 'BlockStatement' || node.type === 'Program' || (node.type === 'FunctionDeclaration' && Array.isArray(node.body))) {
                const targetBody = node.type === 'FunctionDeclaration' ? node.body : node.body;
                
                if (Array.isArray(targetBody)) {
                    const newBody = [];
                    let foundReturn = false;
                    
                    for (const stmt of targetBody) {
                        if (foundReturn) {
                            this.optimizations.push({
                                type: 'Dead Code Elimination',
                                description: 'Removed unreachable code after return statement'
                            });
                        } else {
                            const optimizedStmt = visitor(stmt);
                            if (optimizedStmt) {
                                newBody.push(optimizedStmt);
                                if (optimizedStmt.type === 'ReturnStatement') {
                                    foundReturn = true;
                                }
                            }
                        }
                    }
                    
                    if (node.type === 'FunctionDeclaration') {
                        node.body = newBody;
                    } else {
                        node.body = newBody;
                    }
                }
            } else if (node.type === 'VariableDeclaration' && !node.value) {
                // Check if variable is used
                // Simplified: assume all declared variables are used
            } else if (node.body) {
                if (Array.isArray(node.body)) {
                    node.body = node.body.map(stmt => visitor(stmt)).filter(stmt => stmt !== null);
                } else {
                    node.body = visitor(node.body);
                }
            }
            
            return node;
        };
        
        return visitor(node);
    }
    
    strengthReduction(node) {
        const visitor = (node) => {
            if (!node) return node;
            
            if (node.type === 'BinaryExpression') {
                // Replace multiplication by 2 with shift
                if (node.operator === '*' && node.right?.type === 'NumericLiteral' && parseFloat(node.right.value) === 2) {
                    this.optimizations.push({
                        type: 'Strength Reduction',
                        description: 'Replaced multiplication by 2 with left shift'
                    });
                    
                    return {
                        type: 'BinaryExpression',
                        operator: '<<',
                        left: visitor(node.left),
                        right: { type: 'NumericLiteral', value: '1' }
                    };
                }
                
                // Replace division by 2 with shift
                if (node.operator === '/' && node.right?.type === 'NumericLiteral' && parseFloat(node.right.value) === 2) {
                    this.optimizations.push({
                        type: 'Strength Reduction',
                        description: 'Replaced division by 2 with right shift'
                    });
                    
                    return {
                        type: 'BinaryExpression',
                        operator: '>>',
                        left: visitor(node.left),
                        right: { type: 'NumericLiteral', value: '1' }
                    };
                }
                
                node.left = visitor(node.left);
                node.right = visitor(node.right);
            } else if (node.type === 'UpdateExpression') {
                node.argument = visitor(node.argument);
            } else if (node.type === 'MemberExpression') {
                node.object = visitor(node.object);
                if (node.computed) node.property = visitor(node.property);
            } else if (node.body) {
                if (Array.isArray(node.body)) {
                    node.body = node.body.map(stmt => visitor(stmt));
                } else {
                    node.body = visitor(node.body);
                }
            }
            
            return node;
        };
        
        return visitor(node);
    }
    
    getOptimizationSummary() {
        if (this.optimizations.length === 0) {
            return 'No optimizations applied.';
        }
        
        let summary = 'Optimizations Applied:\n';
        const grouped = {};
        
        this.optimizations.forEach(opt => {
            if (!grouped[opt.type]) {
                grouped[opt.type] = [];
            }
            grouped[opt.type].push(opt.description);
        });
        
        for (const [type, descriptions] of Object.entries(grouped)) {
            summary += `\n${type}:\n`;
            descriptions.forEach(desc => {
                summary += `  - ${desc}\n`;
            });
        }
        
        return summary;
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = Optimizer;
}