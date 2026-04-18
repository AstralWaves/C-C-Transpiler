'use strict';

// Transpiled from C-like source (subset). Target: Node.js
// Run: node <this-file.js>
// For scanf: pipe input, e.g. echo "42" | node <this-file.js>

const util = require('util');

function main() {
    let a = 10;
    let _t0 = 4 * 2;
    let _t1 = 3 + _t0;
    let b = _t1;
    console.log(util.format("%d\n", b));
    return 0;
}

// Run entry point
main();
