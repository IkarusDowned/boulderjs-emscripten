import path from 'path';
import createMultiReaderModule from '../../build/multifilereader.js';

function parseArgs() {
    const args = process.argv.slice(2);
    const fileIndex = args.indexOf('--file');
    if (fileIndex !== -1 && fileIndex + 1 < args.length) {
        return args[fileIndex + 1];
    } else {
        console.error("Usage: node reader.mjs --file ./relative/path/to/file");
        process.exit(1);
    }
}

async function main() {
    const filePathArg = parseArgs();
    const cwd = process.cwd();
    const absoluteFilePath = path.resolve(cwd, filePathArg);

    if (!absoluteFilePath.startsWith(cwd)) {
        console.error(`Error: File must be inside current directory (${cwd})`);
        process.exit(1);
    }

    const relativePath = path.relative(cwd, absoluteFilePath);
    const mountPath = `/input`;
    const wasmFilePath = `${mountPath}/${relativePath.replaceAll('\\', '/')}`;


    const Module = await createMultiReaderModule();

    Module.FS.mkdir(mountPath);
    Module.FS.mount(Module.FS.filesystems.NODEFS, { root: cwd }, mountPath);

    const MultifileReader = Module.MultifileReader;
    const vec = new Module.VectorString();
    vec.push_back(wasmFilePath);
    const reader = new MultifileReader(vec);
    reader.start();

    const poll = () => {
        try {
            const packet = reader.consumePacket();
            if (packet) {
                console.log(packet);
                if (packet === 'EOD') {
                    reader.stop();
                    process.exit(0);
                }
            }
        } catch (e) {
            // Ignore empty queue
            console.log("EMPTY!?!")
        }
        setImmediate(poll);
    };

    poll();

    process.on('SIGINT', () => {
        reader.stop();
        process.exit(0);
    });
}

await main();
