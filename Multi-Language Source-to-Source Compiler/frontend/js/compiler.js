// Compiler state
let currentTarget = 'python';
let currentConversion = null;

document.addEventListener('DOMContentLoaded', function() {
    // Target language buttons
    const targetBtns = document.querySelectorAll('.target-btn');
    targetBtns.forEach(btn => {
        // Set initial active state
        if (btn.dataset.target === currentTarget) {
            btn.classList.add('active');
        }

        btn.addEventListener('click', () => {
            targetBtns.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            currentTarget = btn.dataset.target;
            
            // Auto-compile if code exists
            const sourceCode = getSourceCode();
            if (sourceCode.trim()) {
                handleCompile();
            }
        });
    });
    
    // Compile button
    const compileBtn = document.getElementById('compileBtn');
    if (compileBtn) {
        compileBtn.addEventListener('click', handleCompile);
    }
    
    // Download button
    const downloadBtn = document.getElementById('downloadBtn');
    if (downloadBtn) {
        downloadBtn.addEventListener('click', handleDownload);
    }
    
    // Copy button
    const copyBtn = document.getElementById('copyBtn');
    if (copyBtn) {
        copyBtn.addEventListener('click', handleCopy);
    }
    
    // Tab switching
    const tabBtns = document.querySelectorAll('.tab-btn');
    tabBtns.forEach(btn => {
        btn.addEventListener('click', () => {
            const tabId = btn.dataset.tab;
            
            // Update active tab button
            tabBtns.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            
            // Update active tab content
            document.querySelectorAll('.tab-content').forEach(content => {
                content.classList.remove('active');
            });
            document.getElementById(tabId).classList.add('active');
        });
    });
});

async function handleCompile() {
    const sourceCode = getSourceCode();
    
    if (!sourceCode.trim()) {
        showError('Please enter source code');
        return;
    }
    
    // Show loading state
    showLoading(true);
    updateStatus('compiling', 'Compiling...');
    
    try {
        // Check if server is running
        const isServerUp = await API.checkHealth();
        if (!isServerUp) {
            showError('Laravel server is not running. Please start it using "php artisan serve --port=8000" in the backend folder.');
            updateStatus('error', 'Server offline');
            showLoading(false);
            return;
        }

        const response = await API.compile(sourceCode, currentTarget);
        
        if (response.success) {
            currentConversion = response.data;
            displayResults(response.data);
            updateStatus('success', 'Compilation successful!');
        } else {
            showError(response.message || 'Compilation failed');
            updateStatus('error', 'Compilation failed');
            
            if (response.errors) {
                displayErrors(response.errors);
            }
        }
    } catch (error) {
        console.error('Compilation error:', error);
        showError('Network error. Please try again.');
        updateStatus('error', 'Network error');
    } finally {
        showLoading(false);
    }
}

function displayResults(data) {
    // Display tokens
    const tokensOutput = document.getElementById('tokensOutput');
    if (tokensOutput) {
        tokensOutput.textContent = formatTokens(data.tokens);
    }
    
    // Display AST
    const astOutput = document.getElementById('astOutput');
    if (astOutput) {
        astOutput.textContent = formatAST(data.ast);
    }
    
    // Display intermediate code
    const intermediateOutput = document.getElementById('intermediateOutput');
    if (intermediateOutput) {
        intermediateOutput.textContent = formatIntermediateCode(data.intermediate_code);
    }
    
    // Display optimized code
    const optimizedOutput = document.getElementById('optimizedOutput');
    if (optimizedOutput) {
        optimizedOutput.textContent = data.optimized_code || 'No optimization performed';
    }
    
    // Display target code
    const targetOutput = document.getElementById('targetOutput');
    if (targetOutput) {
        targetOutput.textContent = data.target_code;
    }
    
    // Switch to target code tab
    document.querySelector('.tab-btn[data-tab="target"]').click();
}

function displayErrors(errors) {
    const errorsOutput = document.getElementById('errorsOutput');
    if (errorsOutput) {
        let errorText = 'Compilation Errors:\n\n';
        errors.forEach(error => {
            errorText += `[${error.type.toUpperCase()}] Line ${error.line}: ${error.message}\n`;
            if (error.context) {
                errorText += `  ${error.context}\n`;
            }
        });
        errorsOutput.textContent = errorText;
        
        // Switch to errors tab
        document.querySelector('.tab-btn[data-tab="errors"]').click();
    }
}

function showError(message) {
    const errorsOutput = document.getElementById('errorsOutput');
    if (errorsOutput) {
        errorsOutput.textContent = `Error: ${message}`;
        document.querySelector('.tab-btn[data-tab="errors"]').click();
    }
}

function handleDownload() {
    const targetOutput = document.getElementById('targetOutput');
    if (targetOutput && targetOutput.textContent) {
        const extensions = {
            python: 'py',
            java: 'java',
            javascript: 'js'
        };
        
        const ext = extensions[currentTarget] || 'txt';
        const filename = `converted_code.${ext}`;
        
        const blob = new Blob([targetOutput.textContent], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = filename;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    }
}

async function handleCopy() {
    const targetOutput = document.getElementById('targetOutput');
    if (targetOutput && targetOutput.textContent) {
        try {
            await navigator.clipboard.writeText(targetOutput.textContent);
            showToast('Copied to clipboard!');
        } catch (err) {
            console.error('Copy failed:', err);
        }
    }
}

function showLoading(show) {
    let overlay = document.querySelector('.loading-overlay');
    
    if (show && !overlay) {
        overlay = document.createElement('div');
        overlay.className = 'loading-overlay';
        overlay.innerHTML = '<div class="spinner"></div>';
        document.body.appendChild(overlay);
    } else if (!show && overlay) {
        overlay.remove();
    }
}

function updateStatus(status, message) {
    const indicator = document.querySelector('.status-indicator');
    const statusMsg = document.querySelector('.status-message');
    const statusTime = document.querySelector('.status-time');
    
    if (indicator) {
        indicator.className = 'status-indicator';
        if (status === 'compiling') {
            indicator.classList.add('compiling');
        } else if (status === 'error') {
            indicator.classList.add('error');
        }
    }
    
    if (statusMsg) {
        statusMsg.textContent = message;
    }
    
    if (statusTime) {
        statusTime.textContent = new Date().toLocaleTimeString();
    }
}

function showToast(message) {
    let toast = document.querySelector('.toast');
    
    if (!toast) {
        toast = document.createElement('div');
        toast.className = 'toast';
        document.body.appendChild(toast);
        
        // Add styles for toast
        const style = document.createElement('style');
        style.textContent = `
            .toast {
                position: fixed;
                bottom: 20px;
                right: 20px;
                background: var(--primary);
                color: white;
                padding: 0.8rem 1.5rem;
                border-radius: 0.5rem;
                animation: slideIn 0.3s ease;
                z-index: 3000;
            }
            
            @keyframes slideIn {
                from {
                    transform: translateX(100%);
                    opacity: 0;
                }
                to {
                    transform: translateX(0);
                    opacity: 1;
                }
            }
        `;
        document.head.appendChild(style);
    }
    
    toast.textContent = message;
    toast.style.display = 'block';
    
    setTimeout(() => {
        toast.style.display = 'none';
    }, 3000);
}