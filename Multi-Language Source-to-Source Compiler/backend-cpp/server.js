const express = require('express');
const cors = require('cors');
const multer = require('multer');
const { spawn } = require('child_process');
const fs = require('fs').promises;
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, '../frontend')));

// Configure multer for file uploads
const upload = multer({ dest: path.join(__dirname, 'temp') });

// Ensure temp directory exists
const TEMP_DIR = path.join(__dirname, 'temp');
const COMPILER_PATH = path.join(__dirname, 'build', 'compiler.exe');

// Check if compiler binary exists
async function checkCompiler() {
    try {
        await fs.access(COMPILER_PATH);
        return true;
    } catch {
        return false;
    }
}

// Create temp directory if it doesn't exist
async function ensureTempDir() {
    try {
        await fs.mkdir(TEMP_DIR, { recursive: true });
    } catch (error) {
        console.error('Error creating temp directory:', error);
    }
}

// Get file extension for target language
function getFileExtension(targetLanguage) {
    const extensions = {
        python: 'py',
        java: 'java',
        javascript: 'js',
        cpp: 'cpp'
    };
    return extensions[targetLanguage] || 'txt';
}

// Generate unique filename
function generateUniqueFilename(prefix, extension) {
    return `${prefix}_${Date.now()}_${Math.random().toString(36).substr(2, 8)}.${extension}`;
}

// Clean up temp files
async function cleanupFiles(files) {
    for (const file of files) {
        try {
            await fs.unlink(file).catch(() => {});
        } catch (error) {
            console.error(`Error cleaning up ${file}:`, error);
        }
    }
}

// Parse compiler output
function parseCompilerOutput(output) {
    try {
        const startMarker = '---COMPILER_OUTPUT_START---';
        const endMarker = '---COMPILER_OUTPUT_END---';
        
        const startIndex = output.indexOf(startMarker);
        const endIndex = output.indexOf(endMarker);
        
        if (startIndex !== -1 && endIndex !== -1) {
            const jsonStr = output.substring(
                startIndex + startMarker.length,
                endIndex
            ).trim();
            
            return JSON.parse(jsonStr);
        }
        
        return { success: false, error: 'Invalid compiler output format' };
    } catch (error) {
        return { success: false, error: `Failed to parse compiler output: ${error.message}` };
    }
}

// Run compiler as child process
async function runCompiler(sourceFile, targetLanguage, outputFile) {
    return new Promise((resolve, reject) => {
        const compiler = spawn(COMPILER_PATH, [sourceFile, targetLanguage, outputFile]);
        
        let stdout = '';
        let stderr = '';
        
        compiler.stdout.on('data', (data) => {
            stdout += data.toString();
        });
        
        compiler.stderr.on('data', (data) => {
            stderr += data.toString();
        });
        
        compiler.on('close', async (code) => {
            if (code !== 0) {
                reject(new Error(stderr || `Compiler exited with code ${code}`));
            } else {
                resolve({ stdout, stderr });
            }
        });
        
        compiler.on('error', (error) => {
            reject(new Error(`Failed to start compiler: ${error.message}`));
        });
    });
}

// Health check endpoint
app.get('/api/health', async (req, res) => {
    const compilerExists = await checkCompiler();
    res.json({
        status: 'ok',
        compiler: compilerExists ? 'available' : 'not found',
        timestamp: new Date().toISOString()
    });
});

// API info endpoint
app.get('/api', (req, res) => {
    res.json({
        name: 'C-Transpiler API',
        version: '1.0.0',
        endpoints: {
            'GET /api': 'API information',
            'GET /api/health': 'Health check',
            'GET /api/languages': 'Get supported languages',
            'POST /api/compile': 'Compile C/C++ code'
        }
    });
});

// Compilation endpoint
app.post('/api/compile', async (req, res) => {
    const { code, targetLanguage } = req.body;
    
    if (!code || !code.trim()) {
        return res.status(400).json({
            success: false,
            error: 'Source code is required'
        });
    }
    
    if (!targetLanguage) {
        return res.status(400).json({
            success: false,
            error: 'Target language is required'
        });
    }
    
    const compilerExists = await checkCompiler();
    if (!compilerExists) {
        return res.status(500).json({
            success: false,
            error: 'Compiler binary not found. Please build the C++ compiler first.'
        });
    }
    
    await ensureTempDir();
    
    let sourceFile = null;
    let outputFile = null;
    
    try {
        const sourceFilename = generateUniqueFilename('source', 'c');
        const outputFilename = generateUniqueFilename('output', getFileExtension(targetLanguage));
        
        sourceFile = path.join(TEMP_DIR, sourceFilename);
        outputFile = path.join(TEMP_DIR, outputFilename);
        
        await fs.writeFile(sourceFile, code, 'utf8');
        
        const { stdout } = await runCompiler(sourceFile, targetLanguage, outputFile);
        
        const result = parseCompilerOutput(stdout);
        
        if (result.success) {
            try {
                const targetCode = await fs.readFile(outputFile, 'utf8');
                result.target_code = targetCode;
            } catch (error) {
                result.target_code = '// Error reading generated code';
            }
        }
        
        await cleanupFiles([sourceFile, outputFile]);
        
        res.json(result);
        
    } catch (error) {
        console.error('Compilation error:', error);
        
        if (sourceFile) await cleanupFiles([sourceFile]);
        if (outputFile) await cleanupFiles([outputFile]);
        
        res.status(500).json({
            success: false,
            error: error.message || 'Compilation failed'
        });
    }
});

// Get supported languages
app.get('/api/languages', (req, res) => {
    res.json({
        success: true,
        languages: [
            { id: 'python', name: 'Python', extension: 'py', icon: '🐍' },
            { id: 'java', name: 'Java', extension: 'java', icon: '☕' },
            { id: 'javascript', name: 'JavaScript', extension: 'js', icon: '📜' },
            { id: 'cpp', name: 'C++', extension: 'cpp', icon: '⚙️' }
        ]
    });
});

// Start server
async function startServer() {
    await ensureTempDir();
    
    const compilerExists = await checkCompiler();
    
    app.listen(PORT, () => {
        console.log(`
╔═══════════════════════════════════════════════════════════╗
║                                                           ║
║     🚀 C-Transpiler Backend Server Started!              ║
║                                                           ║
║     Server: http://localhost:${PORT}                        ║
║     API:    http://localhost:${PORT}/api                    ║
║                                                           ║
║     Status:                                              ║
║     - Compiler: ${compilerExists ? '✅ Available' : '❌ Not Found'}                           ║
║                                                           ║
╚═══════════════════════════════════════════════════════════╝
        `);
    });
}

startServer().catch(console.error);