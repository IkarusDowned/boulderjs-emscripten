import path from 'path';
import createMultiReaderModule from '../../build/multifilereader.js';

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

let reader = null;

async function main() {
    const argFilePaths = parseArgs();
    const cwd = process.cwd();


    //initialize - module
    const Module = await createMultiReaderModule();
    const mountPath = `/input`;
    Module.FS.mkdir(mountPath);
    Module.FS.mount(Module.FS.filesystems.NODEFS, { root: cwd }, mountPath);

    //Initialize collect path strings
    const mountedPaths = new Module.VectorString();

    argFilePaths.forEach((filepath) => {
        const absoluteFilePath = path.resolve(cwd, filepath);
        if (!absoluteFilePath.startsWith(cwd)) {
            console.error(`Error: File must be inside current directory (${cwd})`);
            process.exit(1);
        }

        const relativePath = path.relative(cwd, absoluteFilePath);
        const wasmFilePath = `${mountPath}/${relativePath.replaceAll('\\', '/')}`;
        mountedPaths.push_back(wasmFilePath);
    });

    //create reader
    const MultifileReader = Module.MultifileReader;
    var awaitingEod = mountedPaths.size();

    reader = new MultifileReader(mountedPaths);
    //start
    const start = Date.now();
    reader.start();
    const poll = () => {
        try {
            if (reader.getPendingPacketCount() > 0) {

                const packet = reader.consumePacket();
                if (packet) {

                    if (packet.includes('EOD')) {
                        awaitingEod -= 1;
                        if (awaitingEod <= 0) {
                            const elapsed = Date.now() - start;
                            console.log(`${elapsed}ms`);
                            reader.stop();
                            process.exit(0);
                        }

                    }
                    setImmediate(poll); //immediately check if there is a new packet
                    //setTimeout(poll, );
                    return;
                }
            }
        } catch (e) {
            //this should never be reached!
            console.log(e)
            process.exit(1);
        }
        //console.log("timeout from no read...");
        setTimeout(poll, 5);
    };

    poll();
    
    process.on('SIGINT', () => {
        if (reader) {
            reader.stop();

        }
        process.exit(0);
    });

}

await main();
