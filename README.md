## How to build

### Multi File Reader
`docker build -t multifilereader -f multifilereader.dockerfile .`

## How to run
### Create a data file with "packets"
` node ./js/writer.js --file <filename>`

### Multi file Reader
`docker run -v "<input folder>:/var/input" multifilereader --file <filename>`

**Note:** filename gets appended to /var/output/

D:\Projects\boulderjs-emscripten\output