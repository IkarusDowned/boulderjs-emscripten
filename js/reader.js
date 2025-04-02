const fs = require('fs');
const path = require('path');

// STX and ETX ASCII characters
const STX = '\x02';
const ETX = '\x03';

let buffer = '';
let inPacket = false;
let filePosition = 0;

const args = process.argv.slice(2);
const fileIndex = args.indexOf('--file');

if (fileIndex === -1 || !args[fileIndex + 1]) {
  console.error('Usage: node packet_reader.js --file <filepath>');
  process.exit(1);
}

const filePath = path.resolve("/var/input/" + args[fileIndex + 1]);

// Start the timer
const startTime = Date.now();

function printElapsedAndExit(reason) {
  const elapsed = Date.now() - startTime;
  console.log(`${reason}. Elapsed time: ${elapsed} ms`);
  process.exit();
}

// Read new data from current file position
function readNewData() {
  fs.open(filePath, 'r', (err, fd) => {
    if (err) {
      console.error('Error opening file:', err.message);
      return;
    }

    fs.fstat(fd, (err, stats) => {
      if (err) {
        console.error('Error stating file:', err.message);
        fs.close(fd, () => {});
        return;
      }

      const bytesToRead = stats.size - filePosition;

      if (bytesToRead <= 0) {
        fs.close(fd, () => {});
        return; // no new data
      }

      const bufferData = Buffer.alloc(bytesToRead);

      fs.read(fd, bufferData, 0, bytesToRead, filePosition, (err, bytesRead) => {
        if (err) {
          console.error('Error reading file:', err.message);
          fs.close(fd, () => {});
          return;
        }

        filePosition += bytesRead;
        const chunk = bufferData.toString('utf8');
        processChunk(chunk);
        fs.close(fd, () => {});
      });
    });
  });
}

// Extract and process packets
function processChunk(chunk) {
  for (const char of chunk) {
    if (char === STX) {
      buffer = '';
      inPacket = true;
    } else if (char === ETX && inPacket) {
      //console.log(buffer);
      if (buffer.includes('EOD')) {
        printElapsedAndExit('EOD detected');
      }
      inPacket = false;
    } else if (inPacket) {
      buffer += char;
    }
  }
}

// Watch the file for new data
fs.watchFile(filePath, { interval: 500 }, (curr, prev) => {
  if (curr.size > prev.size) {
    readNewData();
  }
});

// Initial read
readNewData();

// Handle SIGINT (Ctrl+C)
process.on('SIGINT', () => {
  printElapsedAndExit('Interrupted (SIGINT)');
});
