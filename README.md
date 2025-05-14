# Multilevel Feedback Queue Process Scheduler

A C-based process scheduling simulator that implements a **multilevel feedback queue** with both preemptive and non-preemptive modes. This simulator was built for **CSCI 447: Operating Systems** (Spring 2024).

Author: **Andrew Cox**
Date: **May 2024**

## Project Description

The simulator models CPU scheduling using:
- **Queue A**: FCFS (First-Come, First-Served)
- **Queue B**: Priority Scheduling (with preemption support)

Key behaviors:
- New processes enter Queue B by default.
- Processes can be promoted from B → A after 3 consecutive CPU bursts under quantum.
- Processes are dispatched from Queue A before Queue B.
- IO operations and decode times are correctly modeled.

## Features

- Support for preemptive and non-preemptive scheduling
- Custom input format with process instructions
- Accurate modeling of decode time, IO, and execution
- Automatic process promotion based on CPU usage pattern
- Real-time simulation and statistics tracking
- Clean CLI output for debugging and analysis

## File Structure

- `Simulation.c/h`: Main entry point and simulation logic
- `queue.c/h`: Data structures and operations for tasks and processes
- `parser.c/h`: Parses structured input file into simulation-ready processes
- `Makefile`: Builds the simulator
- `sampleInputFile1.txt`: Example simulation input

## Building the Project

Run the following in the project directory:

```bash
make
```

This will produce an executable called `Simulation`.

## Running the Simulation

```bash
./Simulation <input-file> <quantumA> <quantumB> <preemption>
```

- `<input-file>`: A `.txt` file containing process definitions (see below)
- `<quantumA>`: Integer quantum for Queue A
- `<quantumB>`: Integer quantum for Queue B
- `<preemption>`: `1` to enable preemption, `0` for non-preemptive mode

### Example:

```bash
./Simulation sampleInputFile1.txt 5 10 1
```

## Input File Format

Each process includes:

- A process ID (PID) and priority
- Arrival time
- A sequence of CPU (`exe:<time>`) and I/O (`io:<time>`) instructions
- `terminate` instruction at the end

### Sample:
```txt
P1003:67
arrival_t:7
exe:6
terminate
```

## Output

Simulation output includes:

- Start and end time
- Number of completed processes and instructions
- Average, min, and max wait times
- Completion summary for each process

## Analysis

This project supports the analysis of optimal quantum values and scheduling strategies. Use various input files and combinations of quantumA, quantumB, and preemption flags to evaluate:

- Throughput
- Wait time distribution
- Impact of promotion and preemption
- Fairness across processes

## Cleanup

To remove compiled binaries:
```bash
make clean
```
