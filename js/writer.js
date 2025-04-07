const fs = require('fs');
const path = require('path');

// STX and ETX ASCII characters
const STX = '\x02';
const ETX = '\x03';

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

function createTimestamp() {
    return Buffer.from(`${STX}${Date.now()}${ETX}`);
}


function createEod() {
    return Buffer.from(`${STX}EOD${ETX}`);
}

function main() {
    const filePath = parseArgs();

    fs.open(filePath, 'w', (err, fd) => {
        if (err) {
            console.error("Failed to open file:", err);
            return;
        }

        process.on('SIGINT', () => {
            const eod = createEod();
            fs.writeSync(fd, eod);
            fs.close(fd, () => {});
            process.exit(0);
        });
    
        const loop = () => {
    
            const ts = createTimestamp();
            fs.writeSync(fd, ts);
            setImmediate(loop);
    
        }
        loop();
    })
    

    
}

main();