// API Configuration
const API_BASE_URL = 'http://127.0.0.1:8000/api';
let authToken = localStorage.getItem('auth_token');

// API Service
const API = {
    // Health check
    async checkHealth() {
        try {
            const response = await fetch(`${API_BASE_URL}/up`, {
                method: 'GET',
                headers: { 'Accept': 'application/json' }
            });
            return response.ok;
        } catch (error) {
            console.error('Server health check failed:', error);
            return false;
        }
    },
    // Authentication
    async login(email, password) {
        const response = await fetch(`${API_BASE_URL}/login`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Accept': 'application/json'
            },
            body: JSON.stringify({ email, password })
        });
        return response.json();
    },
    
    async register(name, email, password) {
        const response = await fetch(`${API_BASE_URL}/register`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Accept': 'application/json'
            },
            body: JSON.stringify({ name, email, password })
        });
        return response.json();
    },
    
    // Compiler endpoints
    async compile(sourceCode, targetLanguage) {
        const response = await fetch(`${API_BASE_URL}/compile`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Accept': 'application/json',
                'Authorization': authToken ? `Bearer ${authToken}` : ''
            },
            body: JSON.stringify({
                source_code: sourceCode,
                target_language: targetLanguage,
                source_language: 'c'
            })
        });
        return response.json();
    },
    
    // History endpoints
    async getHistory() {
        const response = await fetch(`${API_BASE_URL}/history`, {
            headers: {
                'Authorization': authToken ? `Bearer ${authToken}` : ''
            }
        });
        return response.json();
    },
    
    async getConversion(id) {
        const response = await fetch(`${API_BASE_URL}/conversion/${id}`, {
            headers: {
                'Authorization': authToken ? `Bearer ${authToken}` : ''
            }
        });
        return response.json();
    }
};

// Helper function to format output
function formatTokens(tokens) {
    if (!tokens || tokens.length === 0) return 'No tokens generated';
    
    let output = '┌────────────┬─────────────┬──────────┬──────────┐\n';
    output += '│ Line       │ Type        │ Value    │ Column   │\n';
    output += '├────────────┼─────────────┼──────────┼──────────┤\n';
    
    tokens.forEach(token => {
        output += `│ ${String(token.line || 1).padEnd(10)} │ ${String(token.type || 'UNKNOWN').padEnd(11)} │ ${String(token.value || '').padEnd(8)} │ ${String(token.column || 0).padEnd(8)} │\n`;
    });
    
    output += '└────────────┴─────────────┴──────────┴──────────┘';
    return output;
}

function formatAST(ast) {
    if (!ast) return 'No AST generated';
    return JSON.stringify(ast, null, 2);
}

function formatIntermediateCode(code) {
    if (!code || code.length === 0) return 'No intermediate code generated';
    
    let output = '';
    code.forEach((instruction, index) => {
        output += `${index.toString().padStart(4)}: ${instruction}\n`;
    });
    return output;
}