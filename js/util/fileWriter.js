const fs = require('fs');
const path = require('path');

// STX and ETX ASCII characters
const STX = '\x02';
const ETX = '\x03';

function parseArgs() {
    const args = process.argv.slice(2);
    const fileIndex = args.indexOf('--file');
    const packetsIndex = args.indexOf('--packets');

    if (fileIndex === -1 || fileIndex + 1 >= args.length) {
        console.error("Usage: node writer.js --file <filename> [--packets <number>]");
        process.exit(1);
    }

    const filePath = args[fileIndex + 1];
    let packets = Infinity;

    if (packetsIndex !== -1 && packetsIndex + 1 < args.length) {
        packets = parseInt(args[packetsIndex + 1], 10);
        if (isNaN(packets) || packets < 1) {
            console.error("Invalid value for --packets. Must be a positive integer.");
            process.exit(1);
        }
    }

    return { filePath, packets };
}

function createTimestamp(fileName) {
    return Buffer.from(`${STX}${fileName}:${Date.now()}${ETX}`);
}


function createEod(fileName) {
    return Buffer.from(`${STX}${fileName}:EOD${ETX}`);
}

function main() {
    const { filePath, packets } = parseArgs();
    const fileName = path.basename(filePath);
    let writtenPackets = 0;
    fs.open(filePath, 'w', (err, fd) => {
        if (err) {
            console.error("Failed to open file:", err);
            return;
        }

        process.on('SIGINT', () => {
            const eod = createEod(fileName);
            fs.writeSync(fd, eod);
            fs.close(fd, () => { });
            process.exit(0);
        });

        const loop = () => {
            if (writtenPackets >= packets) {
                const eod = createEod(fileName);
                fs.writeSync(fd, eod);
                fs.close(fd, () => { });
                process.exit(0);
            }
            const ts = createTimestamp(fileName);
            fs.writeSync(fd, ts);
            writtenPackets += 1;
            setImmediate(loop);

        }
        loop();
    })



}

main();