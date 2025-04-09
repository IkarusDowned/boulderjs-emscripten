import fs from 'fs/promises';

const STX = 0x02;
const ETX = 0x03;

export default class Producer {
    constructor(filePath, sharedBuffer) 
    {
        this.filePath = filePath;
        this.sharedBuffer = sharedBuffer;
        this.chunkSize = 64;
        this.fileHandle = null;
        this.offset = 0;
        this.leftover = Buffer.alloc(0);
        this.finished = false;
    }

    async initialize() 
    {
        this.fileHandle = await fs.open(this.filePath, 'r');
    }

    async run() 
    {
        const chunk = Buffer.alloc(this.chunkSize);

        while (!this.finished) {
            const { bytesRead } = await this.fileHandle.read(chunk, 0, this.chunkSize, this.offset);
            if (bytesRead <= 0) break;

            this.offset += bytesRead;
            let data = Buffer.concat([this.leftover, chunk.subarray(0, bytesRead)]);

            let stxIndex = data.indexOf(STX);
            while (stxIndex !== -1) {
                const etxIndex = data.indexOf(ETX, stxIndex + 1);
                if (etxIndex === -1) break;

                const packet = data.subarray(stxIndex + 1, etxIndex);
                const packetStr = packet.toString('utf-8');
                this.sharedBuffer.push(packetStr);

                data = data.subarray(etxIndex + 1);
                stxIndex = data.indexOf(STX);
            }

            this.leftover = data;

            await new Promise(resolve => setImmediate(resolve));
        }

        await this.fileHandle.close();
    }

    stop()
    {
        this.finished = true;
    }

}