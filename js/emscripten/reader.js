import createMultiReaderModule from '../../build/multifilereader.js';
const fs = require('fs');
const path = require('path');

function parseArgs() {
    const args = process.argv.slice(2);
    const fileIndex = args.indexOf('--file');
    if (fileIndex !== -1 && fileIndex + 1 < args.length) {
        return args[fileIndex + 1];
    } else {
        console.error("Usage: node writer.js --file <filename>");
        process.exit(1);
    }
}

function main() {
    const filePath = parseArgs();
    createMultiReaderModule().then(Module => {
        Module.FS.mkdir()
    });

    
}

main();