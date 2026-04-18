'use strict';

// Transpiled from C-like source (subset). Target: Node.js
// Run: node <this-file.js>
// For scanf: pipe input, e.g. echo "42" | node <this-file.js>

const util = require('util');

function main() {
    let x = 42;
    console.log(util.format("Value: %d\n", x));
    return 0;
}

// Run entry point
main();
