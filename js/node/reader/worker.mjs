import { parentPort, workerData } from 'worker_threads';
import fs from 'fs';
import { Welford } from './welford.mjs';

const STX = '\x02';
const ETX = '\x03';
const POLL_INTERVAL_MS = 20; // Adjust based on expected data rate

const file = workerData.file;

const priceWelford = new Welford(1.0, 0.5, 4.0);
const volumeWelford = new Welford(2, 1, 3);
let buffer = '';
let lastSize = 0;

const pollFile = () => {
  fs.stat(file, (err, statsObj) => {
    if (err) {
      console.error(`Error polling file ${file}:`, err);
      return;
    }

    if (statsObj.size > lastSize) {
      const stream = fs.createReadStream(file, {
        encoding: 'utf-8',
        start: lastSize,
        end: statsObj.size - 1
      });

      stream.on('data', chunk => {
        buffer += chunk;

        let start, end;
        while ((start = buffer.indexOf(STX)) !== -1 && (end = buffer.indexOf(ETX, start)) !== -1) {
          const packet = buffer.substring(start + 1, end);
          buffer = buffer.substring(end + 1);
          const [filename, type, valueStr, timestamp] = packet.split(':');
          
          let spikeData = null;
          
          if (type === "V") {
            const value = parseInt(valueStr);
            if (isNaN(value)) continue;
            const spike = volumeWelford.updateAndDetectSpike(value);
            
            if (spike != null) {
              const mean = Math.round(Number(spike.mean));
              const stddev = Math.round(Number(spike.stddev));
              spikeData =
              {
                type: 'volume',
                file,
                data: { filename, type, value: spike.value, stddev, mean, timestamp }
              };
            }
          }
          if (type === "P") {
            const value = parseFloat(valueStr);
            if (isNaN(value)) continue;
            const spike = priceWelford.updateAndDetectSpike(value);

            if (spike != null) {
              const mean = Number(spike.mean).toFixed(4);
              const stddev = Number(spike.stddev).toFixed(4);
              spikeData =
              {
                type: 'price',
                file,
                data: { filename, type, value: spike.value, stddev, mean, timestamp }
              };
            }
          }
          
          if (spikeData) {
            parentPort.postMessage(spikeData);
          }
          
        }
      });

      stream.on('end', () => {
        lastSize = statsObj.size;
      });
    }
  });
};

// Start polling
setInterval(pollFile, POLL_INTERVAL_MS);
//setImmediate(pollFile);
