/*
 * Simulation.h
 *
 * Author: Andrew Cox
 * Date: 04 May 2024
 *
 * This file contains the Simulation class, which is responsible for running
 * the simulation.
 */

 #ifndef SIMULATION_H
 #define SIMULATION_H

 #ifndef INT_MAX
 #define INT_MAX 2147483647
 #endif

 #include <stdio.h>
 #include "queue.h"

 // Struct for the simulation
 typedef struct Simulation {
     FILE *input_file;  // file pointer for input file
     int quantumA;      // quantum for queueA
     int quantumB;      // quantum for queueB
     int preemption;    // flag for preemption
     int CPU;           // flag for CPU availability
     int start;         // flag for start of simulation
     int end;           // flag for end of simulation
     pQueue *queueB;    // main process queue
 } Simulation;

 // Struct for the statistics
 typedef struct Stats {
     int instructions;  // total number of instructions
     int startTime;     // start time of simulation
     int runtime;       // total runtime of simulation
     int maxWait;       // maximum wait time
     int minWait;       // minimum wait time
     float totalWait;   // total wait time
 } Stats;

 // function prototypes
 void Simulate(int quantumA, int quantumB, int preemption, pQueue *queueB);
 Stats *initializeStats();
 int allQueuesEmpty(pQueue *queueA, pQueue *queueB, tQueue *readyQueueA, tQueue *readyQueueB, tQueue *ioQueue);
 void runPreemption(int quantumA, int quantumB, pQueue *queueB);
 void runNonPreemption(int quantumA, int quantumB, pQueue *queueB);
 void printStats(pQueue *exitQueue, Stats *stats);
 int main(int argc, char *argv[]);

 #endif
