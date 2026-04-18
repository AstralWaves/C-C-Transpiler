// UI Controller for managing the interface
class UIController {
    constructor() {
        this.sourceEditor = null;
        this.outputEditor = null;
        this.phases = {
            1: { element: 'phase1', status: 'phase1Status', output: 'phase1Output' },
            2: { element: 'phase2', status: 'phase2Status', output: 'phase2Output' },
            3: { element: 'phase3', status: 'phase3Status', output: 'phase3Output' },
            4: { element: 'phase4', status: 'phase4Status', output: 'phase4Output' },
            5: { element: 'phase5', status: 'phase5Status', output: 'phase5Output' },
            6: { element: 'phase6', status: 'phase6Status', output: 'phase6Output' }
        };
    }
    
    initializeEditors() {
        // Initialize Ace Editor for source code
        this.sourceEditor = ace.edit('sourceEditor');
        this.sourceEditor.setTheme('ace/theme/monokai');
        this.sourceEditor.session.setMode('ace/mode/c_cpp');
        this.sourceEditor.setOptions({
            fontSize: '14px',
            showPrintMargin: false,
            showGutter: true,
            highlightActiveLine: true,
            enableBasicAutocompletion: true,
            enableLiveAutocompletion: true,
            useSoftTabs: true,
            tabSize: 4
        });
        
        // Set default C++ code
        this.sourceEditor.setValue(`#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int main() {
    int x = 10;
    int y = 20;
    int result;
    
    result = add(x, y);
    
    if (result > 25) {
        result = result * 2;
    }
    
    for (int i = 0; i < 5; i++) {
        result = result + i;
    }
    
    return result;
}`, -1);
        
        // Initialize Ace Editor for output code
        this.outputEditor = ace.edit('outputEditor');
        this.outputEditor.setTheme('ace/theme/monokai');
        this.outputEditor.session.setMode('ace/mode/java');
        this.outputEditor.setOptions({
            fontSize: '14px',
            showPrintMargin: false,
            showGutter: true,
            highlightActiveLine: true,
            readOnly: true,
            useSoftTabs: true,
            tabSize: 4
        });
        
        this.outputEditor.setValue('// Generated code will appear here...');

        // Add resize observer to handle container resizing
        const resizeObserver = new ResizeObserver(() => {
            this.sourceEditor.resize();
            this.outputEditor.resize();
        });
        
        resizeObserver.observe(document.getElementById('sourceEditor'));
        resizeObserver.observe(document.getElementById('outputEditor'));
    }
    
    attachEventListeners() {
        // Modal events
        const infoBtn = document.getElementById('infoBtn');
        const infoModal = document.getElementById('infoModal');
        const closeModal = document.getElementById('closeModal');
        
        if (infoBtn && infoModal && closeModal) {
            infoBtn.addEventListener('click', () => {
                infoModal.classList.add('active');
            });
            
            closeModal.addEventListener('click', () => {
                infoModal.classList.remove('active');
            });
            
            window.addEventListener('click', (e) => {
                if (e.target === infoModal) {
                    infoModal.classList.remove('active');
                }
            });
        }

        // Clear button
        const clearBtn = document.getElementById('clearBtn');
        if (clearBtn) {
            clearBtn.addEventListener('click', () => {
                if (window.app) window.app.clear();
            });
        }
    }
    
    updatePhaseStatus(phase, status, output = null) {
        const phaseData = this.phases[phase];
        if (!phaseData) return;
        
        const card = document.getElementById(phaseData.element);
        const statusElement = document.getElementById(phaseData.status);
        const outputElement = document.getElementById(phaseData.output);
        const progressFill = document.getElementById('compilationProgress');
        
        // Remove existing status classes
        card.classList.remove('active', 'completed', 'error');
        statusElement.classList.remove('completed', 'processing', 'error');
        
        // Update status
        statusElement.textContent = status;
        
        switch (status) {
            case 'Processing':
                card.classList.add('active');
                statusElement.classList.add('processing');
                // Scroll into view if it's the active phase
                card.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
                break;
            case 'Completed':
                card.classList.add('completed');
                statusElement.classList.add('completed');
                // Update overall progress
                if (progressFill) {
                    const progress = (phase / 6) * 100;
                    progressFill.style.width = `${progress}%`;
                }
                break;
            case 'Error':
                card.classList.add('error');
                statusElement.classList.add('error');
                if (progressFill) {
                    progressFill.style.background = 'var(--danger-color)';
                }
                break;
            default:
                statusElement.textContent = 'Pending';
        }
        
        // Update output
        if (output !== null) {
            const pre = outputElement.querySelector('pre') || document.createElement('pre');
            pre.textContent = output;
            if (!outputElement.contains(pre)) {
                outputElement.appendChild(pre);
            }
        }
    }
    
    clearPhases() {
        const progressFill = document.getElementById('compilationProgress');
        if (progressFill) {
            progressFill.style.width = '0%';
            progressFill.style.background = 'linear-gradient(90deg, var(--primary-color), var(--secondary-color))';
        }
        for (let i = 1; i <= 6; i++) {
            this.updatePhaseStatus(i, 'Pending', 'Waiting for input...');
        }
    }
    
    updateOutputLanguage(language) {
        const badge = document.getElementById('outputLangBadge');
        const modeMap = {
            'java': 'ace/mode/java',
            'javascript': 'ace/mode/javascript',
            'csharp': 'ace/mode/csharp'
        };
        
        const extensionMap = {
            'java': '.java',
            'javascript': '.js',
            'csharp': '.cs'
        };
        
        badge.textContent = language.charAt(0).toUpperCase() + language.slice(1);
        this.outputEditor.session.setMode(modeMap[language]);
        
        const panelHeader = document.querySelector('.output-panel .file-badge');
        if (panelHeader) {
            panelHeader.textContent = `output${extensionMap[language]}`;
        }
    }
    
    addConsoleMessage(message, type = 'info') {
        const consoleOutput = document.getElementById('consoleOutput');
        const messageElement = document.createElement('div');
        
        messageElement.className = `console-${type}`;
        messageElement.innerHTML = `<i class="fas fa-${this.getIconForType(type)}"></i> ${message}`;
        
        consoleOutput.appendChild(messageElement);
        consoleOutput.scrollTop = consoleOutput.scrollHeight;
    }
    
    getIconForType(type) {
        const icons = {
            'error': 'times-circle',
            'warning': 'exclamation-triangle',
            'success': 'check-circle',
            'info': 'info-circle'
        };
        return icons[type] || 'info-circle';
    }
    
    clearConsole() {
        document.getElementById('consoleOutput').innerHTML = '';
    }
    
    showCompileTime(time) {
        const timeElement = document.getElementById('compileTime');
        timeElement.textContent = `Compiled in ${time}ms`;
    }
    
    setOutputCode(code) {
        this.outputEditor.setValue(code, -1);
    }
    
    getSourceCode() {
        return this.sourceEditor.getValue();
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = UIController;
}