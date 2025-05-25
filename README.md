# myscheduler

A C-based simulation of an operating system process scheduler, supporting CPU scheduling, device I/O, and process synchronization. Developed for CITS2002 Project 1 (2023).

---

## Summary

This project implements a process scheduler simulation in C, designed for the CITS2002 course (Project 1, 2023). The scheduler simulates the execution of processes and their system calls, managing CPU, device I/O, and process synchronization (sleep, wait, spawn) according to a configuration and command specification. The simulation models real-world scheduling concepts such as time quantums, context switches, device data rates, and process blocking, providing a platform for understanding operating system process management.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Build & Run](#build--run)
- [Configuration Files](#configuration-files)
- [How It Works](#how-it-works)
- [Authors](#authors)
- [License](#license)

---

## Overview

`myscheduler` is a simulation tool that models the behavior of an operating system's process scheduler. It reads a system configuration file and a command file, then simulates the execution of processes, including their system calls (CPU, I/O, sleep, spawn, wait, exit). The simulation tracks process states, device usage, and timing, providing insights into scheduling algorithms and process management.

---

## Features

- **CPU Scheduling**: Simulates round-robin scheduling with configurable time quantum and context switch overhead.
- **Device I/O**: Models multiple devices with configurable read/write speeds and simulates data transfer delays.
- **Process Synchronization**: Supports process spawning, waiting for child processes, and sleeping.
- **Event-Driven Simulation**: Advances a global clock to the next event (CPU, I/O, or blocked process ready).
- **Configurable**: Reads system and command configurations from external files.
- **Statistics Output**: (Planned) Outputs measurements such as total completion time and device utilization.

---

## Project Structure

```
cits2002-project1/
├── myscheduler.c         # Main scheduler implementation
├── test.c               # Test code for data structures
├── command-file         # Example command file
├── sysconfig-file       # Example system configuration file
├── test-commands.txt    # Test command file
├── test-sysconfig.txt   # Test system configuration file
├── Notes.txt            # Project notes and design questions
├── Josh-Implementation.txt # Design notes and pseudocode
├── [other files...]
```

---

## Build & Run

### Requirements

- C compiler supporting C11 (e.g., `gcc`, `clang`)
- Unix-like environment (tested on macOS)

### Compilation

```sh
cc -std=c11 -Wall -Werror -o myscheduler myscheduler.c
```

### Usage

```sh
./myscheduler sysconfig-file command-file
```

- `sysconfig-file`: Path to the system configuration file (defines devices and time quantum)
- `command-file`: Path to the command file (defines processes and their system calls)

---

## Configuration Files

### System Configuration (`sysconfig-file`)

Defines available devices and their data rates, as well as the scheduler's time quantum.

Example:
```
#            devicename   readspeed      writespeed
#
device       usb3.1       640000000Bps   640000000Bps
device       terminal     10Bps          3000000Bps
device       hd           160000000Bps   80000000Bps
device       ssd          480000000Bps   420000000Bps
#
timequantum  100usec
```

### Command File (`command-file`)

Defines processes (commands) and their system calls.

Example:
```
#
shortsleep
    10usecs    sleep   1000000usecs
    50usecs    exit
#
cal
    80usecs    write   terminal 2000B
    90usecs    exit
#
```

---

## How It Works

- **Initialization**: Reads device and process definitions from configuration files.
- **Queues**: Maintains ready, data (per device), and blocked queues for process management.
- **Global Clock**: Advances to the next event (CPU, I/O, or blocked process ready).
- **System Calls**: Handles `read`, `write`, `sleep`, `spawn`, `wait`, and `exit` system calls, updating process states and queues accordingly.
- **Device Simulation**: Simulates data transfer delays based on device speeds.
- **Synchronization**: Supports process spawning and waiting for child processes to finish.
- **Statistics**: (Planned) Outputs measurements such as total completion time and device utilization.

---

## Authors

- Adriaan van der Berg (23336556)
- Joshua Then (23432725)

---

## License

This project is for educational purposes as part of the CITS2002 course at UWA. 