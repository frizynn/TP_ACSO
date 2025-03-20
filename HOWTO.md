# ARMv8 Simulator - HOWTO Guide

This guide explains how to build, run, and test the ARMv8 simulator using Docker.

## Prerequisites

- Docker installed on your system
- The project files in the correct directory structure

## Building the Docker Image

1. Ensure you are in the root directory of the project where the Dockerfile is located.
2. Build the Docker image:

```bash
docker build -t armv8-simulator .
```

## Running the Simulator

### 1. Start the Docker Container

```bash
docker run --rm -it armv8-simulator
```

This will start an interactive shell inside the container.

### 2. Compile the Simulator

Once inside the container:

```bash
cd src
make clean  # Optional: Clean previous builds
make
```

### 3. Run the Simulator with a Test File

The simulator can run bytecode files located in the `inputs/bytecodes` directory:

```bash
./sim ../inputs/bytecodes/eor.x
```

Available bytecode files include:
- adds.x
- addis.x
- adds-subs.x
- ands.x
- beq.x
- blt.x
- eor.x
- movz.x
- subis.x
- sturb.x

### 4. Using the Simulator Shell

Once the simulator is running, you can use the following commands:

- `go`: Execute the program completely until halt
- `run <n>`: Execute n instructions
- `mdump <low> <high>`: Memory dump from low to high address
- `rdump`: Register dump
- `input reg_num reg_val`: Set a register value
- `?`: Display help
- `quit`: Exit the simulator

Example session:
```
ARM-SIM> go
Simulating...
Simulator halted

ARM-SIM> rdump
```

## Running the Reference Simulator

The project includes reference simulators for comparison. From the main directory:

```bash
./ref_sim_x86 inputs/bytecodes/addis.x
```

Note: The reference simulator filename depends on your host architecture (ref_sim_x86 for x86 systems).

## Compiling Your Own Assembly Code

If you have your own ARMv8 assembly code (.s files), you can compile it using the provided tools:

```bash
cd inputs
./asm2hex your_code.s > bytecodes/your_code.x
```

Then run it with the simulator:

```bash
cd ../src
./sim ../inputs/bytecodes/your_code.x
```

## Troubleshooting

If you see "Exec format error" when trying to run the simulator, it means the binary was compiled for a different architecture. Make sure to:

1. Run `make clean` to remove the old binary
2. Run `make` to rebuild for your current architecture
3. Try running the simulator again 