FROM ubuntu:22.04

# Install required dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    make \
    gdb \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /app

# Copy the ARMv8 simulator project
COPY TP1-ARM/ ./

# Set the default command to open a shell
CMD ["/bin/bash"] 