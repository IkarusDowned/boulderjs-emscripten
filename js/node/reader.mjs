import path from 'path';
import Producer from './producer.mjs';

function parseArgs() {
    const args = process.argv.slice(2);
    const fileArgIndex = args.indexOf('--files');
    if (fileArgIndex !== -1 && fileArgIndex + 1 < args.length) {
        const rawPaths = args[fileArgIndex + 1]
            .split(',')
            .map(p => path.resolve(process.cwd(), p.trim()));
        return rawPaths;
    } else {
        console.error("Usage: node reader.mjs --files file1,file2,file3");
        process.exit(1);
    }
}

async function main() {
    const paths = parseArgs();
    const sharedBuffer = [];

    const producers = paths.map((path) => {
        return new Producer(path, sharedBuffer);
    })

    await Promise.all(producers.map(p => p.initialize()));
    producers.forEach(p => p.run());
    let numFunfinishedProducers = producers.length;

    const start = Date.now();
    while (numFunfinishedProducers > 0) {
        if (sharedBuffer.length > 0) {
            
            const packet = sharedBuffer.shift();
            if(packet.includes('EOD'))
            {
                --numFunfinishedProducers;
            }
            

        } else {
            // If buffer is empty, yield
            await new Promise(resolve => setImmediate(resolve));
        }
    }
    const difference = Date.now() - start;
    console.log(`${difference}`);

}

await main();