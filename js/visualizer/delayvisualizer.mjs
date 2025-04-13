export class DelayVisualizer {
    #delays = {};
    #delayScale = 1;

    constructor(measuredItems, delayScale = 2) {
        measuredItems.forEach(item => this.#delays[item] = 0);
        this.#delayScale = delayScale;
    }

    #buildVisualization(key) {

        const scaledDelay = this.#delays[key];
        const delayVisual = '='.repeat(scaledDelay);
        const format = `[${key}]: ${delayVisual}\n`
        return format;
    }

    update(item, delay) {
        
        if (item in this.#delays) {
            const scaledDelay = Math.max(Math.floor(delay / this.#delayScale), 1);
            this.#delays[item] = scaledDelay;
        }

    }

    visualize() {
        //process.stdout.write('\x1B[2J\x1B[0f');
        const now = new Date();
        let visuals = "";
        for (const key in this.#delays) {
            visuals += this.#buildVisualization(key);
        }
        //const output = visuals;
        
        const output =
            '\x1B[?25l' + // hide cursor
            '\x1B[2J' + //clear screen
            '\x1B[0;0H' + // move to top
            [now] + '\n' +
            visuals; 
            //+ '\x1B[?25h';  // show cursor
        

        process.stdout.write(output);
    }
}
