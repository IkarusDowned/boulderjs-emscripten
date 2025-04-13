import fs from 'fs';
import { stat, createReadStream } from 'fs';
import { fileURLToPath } from 'url';
import { dirname, resolve, basename } from 'path';
import process from 'process';
import { DelayVisualizer } from '../../../visualizer/delayvisualizer.mjs';
import { Welford } from '../algorithm/welford.mjs';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// Parse CLI args
const args = process.argv.slice(2);
const filesFlagIndex = args.indexOf('--files');
if (filesFlagIndex === -1 || !args[filesFlagIndex + 1]) {
  console.error('Usage: node reader.mjs --files file1.txt,file2.txt');
  process.exit(1);
}
const files = args[filesFlagIndex + 1].split(',');

// Constants
const STX = '\x02';
const ETX = '\x03';
const POLL_INTERVAL_MS = 20;

const visualizer = new DelayVisualizer(files);
setInterval(() => visualizer.visualize(), 50);

// Utility
function updateSpike(spike, file) {
  const delay = (Date.now() - spike.timestamp);
  visualizer.update(file, delay);
}

class FileSpikeReader {
  constructor(file) {
    this.file = file;
    this.buffer = '';
    this.lastSize = 0;
    this.priceWelford = new Welford(1.0, 0.5, 4.0);
    this.volumeWelford = new Welford(2, 1, 3);
  }

  async poll() {
    stat(this.file, (err, stats) => {
      if (err || stats.size <= this.lastSize) return;

      const stream = createReadStream(this.file, {
        encoding: 'utf-8',
        start: this.lastSize,
        end: stats.size - 1
      });

      stream.on('data', chunk => {
        this.buffer += chunk;
        let start, end;
        while ((start = this.buffer.indexOf(STX)) !== -1 && (end = this.buffer.indexOf(ETX, start)) !== -1) {
          const packet = this.buffer.substring(start + 1, end);
          this.buffer = this.buffer.substring(end + 1);
          this.processPacket(packet);
        }
      });

      stream.on('end', () => {
        this.lastSize = stats.size;
      });
    });
  }

  processPacket(packet) {
    const [filename, type, valueStr, timestamp] = packet.split(':');
    if (!timestamp) return;

    let value = (type === 'V') ? parseInt(valueStr) : parseFloat(valueStr);
    if (isNaN(value)) return;

    let spike = null;
    if (type === 'V') {
      spike = this.volumeWelford.updateAndDetectSpike(value);
    } else if (type === 'P') {
      spike = this.priceWelford.updateAndDetectSpike(value);
    }

    if (spike) {
      updateSpike({
        filename,
        type,
        value: spike.value,
        stddev: Number(spike.stddev).toFixed(4),
        mean: Number(spike.mean).toFixed(4),
        timestamp: Number(timestamp)
      }, this.file);
    }
  }
}

// Start polling each file
const readers = files.map(f => new FileSpikeReader(f));
const intervals = readers.map(reader => setInterval(() => reader.poll(), POLL_INTERVAL_MS));

// Graceful shutdown
process.on('SIGINT', () => {
  console.log('\nShutting down...');
  intervals.forEach(clearInterval);
  process.exit(0);
});
