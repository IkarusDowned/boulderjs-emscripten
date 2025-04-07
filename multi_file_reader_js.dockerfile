# Use an official Node.js LTS image
FROM node:20-slim

# Create app directory
WORKDIR /usr/src/app

# Copy the script into the container
COPY ./js/reader.js .


# Set the entrypoint to run the script with the file argument
ENTRYPOINT ["node", "reader.js"]
