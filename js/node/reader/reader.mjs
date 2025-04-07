import { Worker } from 'worker_threads';
import { fileURLToPath } from 'url';
import { dirname, resolve, basename } from 'path';
import process from 'process';
import { DelayVisualizer } from '../../visualizer/delayvisualizer.mjs';

// Emulate __dirname in ESM
const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// Parse CLI args: --files file1.txt,file2.txt
const args = process.argv.slice(2);
const filesFlagIndex = args.indexOf('--files');

if (filesFlagIndex === -1 || !args[filesFlagIndex + 1]) {
    console.error('Usage: node index.mjs --files file1.txt,file2.txt');
    process.exit(1);
}

const filesArg = args[filesFlagIndex + 1];
const files = filesArg.split(',');

const visualizer = new DelayVisualizer(files);
setInterval(() => visualizer.visualize(), 50);  //render function

// update spikes on main thread. 
const startTime = Date.now();
const updateSpike = (spike, file) => {
    const delay = (Date.now() - spike.timestamp)
    visualizer.update(file, delay);
    //console.log(`[${file}] Type: ${spike.type} Spike: ${spike.value} Mean: ${spike.mean} StdDev: ${spike.stddev} (delay: ${delay} ms)`);
};

// Spawn a worker per file
files.forEach(file => {
    const worker = new Worker(resolve(__dirname, './worker.mjs'), {
        workerData: { file }
    });

    worker.on('message', (msg) => {
       
        updateSpike(msg.data, msg.file);
        
    });

    worker.on('error', err => {
        console.error(`Worker error on file ${file}:`, err);
    });

    worker.on('exit', code => {
        if (code !== 0) {
            console.error(`Worker for file ${file} exited with code ${code}`);
        }
    });
});

// Graceful shutdown on CTRL+C
process.on('SIGINT', () => {
    console.log('\nShutting down...');
    process.exit(0);
});
