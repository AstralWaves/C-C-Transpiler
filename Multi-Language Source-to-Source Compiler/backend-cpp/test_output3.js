'use strict';

// Transpiled from C-like source (subset). Target: Node.js
// Run: node <this-file.js>
// For scanf: pipe input, e.g. echo "42" | node <this-file.js>

const util = require('util');

function main() {
    let _t0 = 4 * 2;
    let _t1 = 3 + _t0;
    let a = _t1;
    let b = 1;
    let _t2 = (a > 10) ? 1 : 0;
    if (_t2) {
        console.log(util.format("%d\n", a));
    } else {
        console.log(util.format("%d\n", b));
    }
    let i = 0;
    while (true) {
        let _t3 = (i < 3) ? 1 : 0;
        if (!(_t3)) break;
        console.log(util.format("%d ", i));
        let _t4 = i + 1;
        i = _t4;
    }
    return 0;
}

// Run entry point
main();
