// CodeMirror editor initialization
let editor;

document.addEventListener('DOMContentLoaded', function() {
    // Initialize CodeMirror editor
    const textarea = document.getElementById('sourceEditor');
    if (textarea) {
        editor = CodeMirror.fromTextArea(textarea, {
            mode: 'text/x-csrc',
            theme: 'dracula',
            lineNumbers: true,
            autoCloseBrackets: true,
            matchBrackets: true,
            indentUnit: 4,
            tabSize: 4,
            lineWrapping: true,
            extraKeys: {
                'Ctrl-Space': 'autocomplete',
                'Ctrl-/': 'toggleComment'
            }
        });
        
        // Load sample code
        const sampleCode = `#include <stdio.h>

int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    int num = 5;
    int result = factorial(num);
    printf("Factorial of %d is %d\\n", num, result);
    
    // Loop example
    for (int i = 0; i < 5; i++) {
        printf("i = %d\\n", i);
    }
    
    return 0;
}`;
        
        editor.setValue(sampleCode);
    }
    
    // Clear button
    const clearBtn = document.getElementById('clearBtn');
    if (clearBtn) {
        clearBtn.addEventListener('click', () => {
            editor.setValue('');
        });
    }
    
    // Load sample button
    const loadSampleBtn = document.getElementById('loadSampleBtn');
    if (loadSampleBtn) {
        loadSampleBtn.addEventListener('click', () => {
            const samples = {
                'simple': `#include <stdio.h>

int main() {
    int x = 10;
    int y = 20;
    int sum = x + y;
    printf("Sum: %d\\n", sum);
    return 0;
}`,
                'loop': `#include <stdio.h>

int main() {
    for (int i = 1; i <= 10; i++) {
        printf("%d ", i);
    }
    printf("\\n");
    return 0;
}`,
                'function': `#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int main() {
    int result = add(5, 3);
    printf("Result: %d\\n", result);
    return 0;
}`
            };
            
            const sample = prompt('Choose sample: simple, loop, function', 'simple');
            if (samples[sample]) {
                editor.setValue(samples[sample]);
            }
        });
    }
    
    // Fullscreen button
    const fullscreenBtn = document.getElementById('fullscreenBtn');
    if (fullscreenBtn) {
        fullscreenBtn.addEventListener('click', () => {
            const editorElement = document.querySelector('.CodeMirror');
            if (editorElement.requestFullscreen) {
                editorElement.requestFullscreen();
            }
        });
    }
});

// Get editor value
function getSourceCode() {
    return editor ? editor.getValue() : '';
}

// Set editor value
function setSourceCode(code) {
    if (editor) {
        editor.setValue(code);
    }
}