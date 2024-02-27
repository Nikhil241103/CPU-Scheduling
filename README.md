# CPU Scheduling Algorithms

This repository contains C implementations of essential CPU scheduling algorithms. These algorithms play a crucial role in operating systems, efficiently managing the execution of processes on a CPU.

## Algorithms Implemented

- **First Come First Serve (FCFS)**
- **Shortest Job First (SJF)**
- **Preemptive Shortest Job First (SJF)**
- **Round Robin (RR)**

## Sample Input and Output Files

- [input.txt](./input.txt): Example input file showcasing process arrival times, burst times, and interrupt flags.
- [output.txt](./output.txt): Example output file demonstrating the results of the CPU scheduling algorithms.

## Description of Each Algorithm

### First Come First Serve (FCFS)
FCFS is a non-preemptive scheduling algorithm that executes processes in the order they arrive, without considering their burst times. It's simple and easy to implement but may lead to long waiting times for processes with long burst times.

### Shortest Job First (SJF)
SJF is a non-preemptive scheduling algorithm that executes the shortest job first. It minimizes average waiting time and turnaround time but requires knowledge of burst times in advance, which may not always be available.

### Preemptive Shortest Job First (SJF)
Preemptive SJF is a preemptive version of the SJF algorithm. It switches to a shorter job if one arrives that requires less CPU time than the currently running job, reducing average waiting time and turnaround time further.

### Round Robin (RR)
RR is a preemptive scheduling algorithm that assigns a fixed time unit (time quantum) to each process in a cyclic manner. It ensures fairness and prevents starvation but may result in higher overhead due to frequent context switches.

## Getting Started

To run the code, compile `cpu_scheduling.c` using a C compiler. Ensure to provide appropriate input in `input.txt`, where process arrival times, burst times, and interrupt flags are specified. The output will be written to `output.txt`.

### Instructions for Compiling and Running

1. Compile the code using a C compiler (e.g., gcc):

2. Run the executable file:

## Testing

To test the code, follow these steps:

1. Create test cases with different sets of input data, considering various scenarios and edge cases.
2. Run the code with each test case and compare the output against expected results.
3. Analyze the output to ensure that the scheduling algorithms function correctly under different conditions.

## Acknowledgments

The code for CPU scheduling algorithms is based on concepts from operating systems textbooks and online resources.
