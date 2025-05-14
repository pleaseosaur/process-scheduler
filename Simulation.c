/*
 *  File: Simulation.c ... a skeleton file
 *  Author: Filip Jagodzinski <filip.jagodzinski@wwu.edu>
 *  Last update : 08 February 2018
 *
 *  Implementation Author: Andrew Cox
 *  Date: 01 May 2024
 */

#include <stdlib.h>
#include <errno.h>

#include "Simulation.h"
#include "parser.h"
#include "queue.h"

/*
 * Function: Simulate
 *
 * Initializes the simulation based on preemption
 */
void Simulate(int quantumA, int quantumB, int preemption, pQueue *queueB) {
    // A function whose input is the quanta for queues A and B,
    // well as whether preemption is enabled.

    if (preemption == 1) {
        runPreemption(quantumA, quantumB, queueB);
    } else {
        runNonPreemption(quantumA, quantumB, queueB);
    }

}

/*
 * Function: initializeStats
 *
 * Initializes the stats struct
 */
Stats *initializeStats() {
    Stats *s = (Stats *)malloc(sizeof(Stats));
    if (!s) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    s->instructions = 0;
    s->startTime = 0;
    s->runtime = 0;
    s->maxWait = 0;
    s->minWait = INT_MAX;
    s->totalWait = 0;

    return s;
}

/*
 * Function: allQueuesEmpty
 *
 * Helper function to check if all queues are empty
 */
int allQueuesEmpty(pQueue *queueA, pQueue *queueB, tQueue *readyQueueA, tQueue *readyQueueB, tQueue *ioQueue) {
    if (isEmptyP(queueB) &&
        isEmptyP(queueA) &&
        isEmptyT(readyQueueA) &&
        isEmptyT(readyQueueB) &&
        isEmptyT(ioQueue)) {
        return 1;
    }
    return 0;
}

/*
 * Function: runPreemption
 *
 * Runs the simulation for preemption scheduling
 */
 void runPreemption(int quantumA, int quantumB, pQueue *queueB) {

     // Initialize simulation variables
     Stats *stats = initializeStats();
     Task *t = NULL;
     Process *p = NULL;
     int cpu = 0;

     // Initialize queues
     pQueue *queueA = createProcessQueue();
     pQueue *exitQueue = createProcessQueue();
     tQueue *ioQueue = createTaskQueue();
     tQueue *readyQueueA = createTaskQueue();
     tQueue *readyQueueB = createTaskQueue();

     // simulation start time == first process arrival time
     p = peekProcess(queueB);
     stats->runtime = stats->startTime = p->arrival;
     p = NULL;

     // main simulation loop for non-preemption
     while (!allQueuesEmpty(queueA, queueB, readyQueueA, readyQueueB, ioQueue)) {

         // prioritize queue A
         if (!isEmptyP(queueA) || !isEmptyT(readyQueueA)) {

             while (!isEmptyP(queueA) || !isEmptyT(readyQueueA)) {

                 // update I/O tasks to simulate concurrent execution
                 if (!isEmptyT(ioQueue)) {
                     updateIOTasks(ioQueue);
                 }

                 if (cpu == 0) {
                     // fetch next task and set CPU flag
                     t = getNextTaskPreemptive(queueA, readyQueueA, stats->runtime);
                     if (t != NULL) {
                         cpu = 1;
                         t->parent->taskRunning = 1;
                     }

                     break;
                 } else {
                     p = t->parent;
                     if (preemptionCheck(queueB, readyQueueB, t, stats->runtime)) {
                         t->interrupts++;
                         p->taskRunning = 0;
                         priorityEnqueueTask(readyQueueB, t);
                         t = getNextTaskPreemptive(queueB, readyQueueB, stats->runtime);
                     } else {
                        switch (t->type) {
                            case 'i':
                                if (p->quantum > 0) { // if process has quantum left
                                    p->quantum--;
                                    if (p->quantum > 0) {
                                        p->completions++;
                                        // p->taskRunning = 0;
                                    } else { // reset completions
                                        p->completions = 0;
                                    }
                                    stats->instructions++;
                                    enqueueTask(ioQueue, t); // add to I/O queue
                                } else {
                                    t->interrupts++;
                                    p->taskRunning = 0;
                                    p->quantum = quantumA;
                                    priorityEnqueueTask(readyQueueA, t);
                                }
                                cpu = 0;
                                break;
                            case 'e':
                                if (t->time == 0) { // task completed
                                    t->completed = 1;
                                    p->taskRunning = 0;
                                    p->currentTask++;
                                    stats->instructions++;
                                    cpu = 0;
                                } else if (p->quantum <= 0) { // quantum used up
                                    p->completions = 0;
                                    t->interrupts++;
                                    p->taskRunning = 0;
                                    p->quantum = quantumA;
                                    priorityEnqueueTask(readyQueueA, t);
                                    cpu = 0;
                                } else {
                                    t->time--;
                                    p->quantum--;
                                }
                                break;
                            default: // 't' - terminate process
                                if (p->quantum > 0) {
                                    p->quantum--;
                                    stats->instructions++;
                                    stats->runtime++;
                                    p->taskRunning = 0;
                                    p->runtime = stats->runtime;
                                    stats->minWait = stats->minWait < p->ready ? stats->minWait : p->ready;
                                    stats->maxWait = stats->maxWait > p->ready ? stats->maxWait : p->ready;
                                    stats->totalWait += p->ready;
                                    endProcess(queueA, exitQueue, p);
                                } else {
                                    p->completions = 0;
                                    t->interrupts++;
                                    p->taskRunning = 0;
                                    p->quantum = quantumA;
                                    priorityEnqueueTask(readyQueueA, t);
                                }
                                cpu = 0;
                                break;
                        }
                     }
                 }

                 // update queueA wait/ready times
                 if (!isEmptyP(queueA)) {
                     updateProcessQueue(queueA, stats->runtime);
                 }

                 // update queueB wait/ready times
                 if (!isEmptyP(queueB)) {
                     updateProcessQueue(queueB, stats->runtime);
                 }

                 stats->runtime++;
             }
         } else { // if queue A is empty

             while (!isEmptyP(queueB) || !isEmptyT(ioQueue) || !isEmptyT(readyQueueB)) {

                 // update I/O queue to simulate concurrent execution
                 if (!isEmptyT(ioQueue)) {
                     updateIOTasks(ioQueue);
                 }

                 if (cpu == 0) {
                     // fetch next task and set CPU flag
                     t = getNextTaskPreemptive(queueB, readyQueueB, stats->runtime);
                     if (t != NULL) {
                         cpu = 1;
                         t->parent->taskRunning = 1;
                     }

                     break;
                 } else {
                     p = t->parent; // identify parent process
                     if (preemptionCheck(queueB, readyQueueB, t, stats->runtime)) {
                         t->interrupts++;
                         p->taskRunning = 0;

                         if (t->interrupts == 3) { // promote to queue A
                             promoteProcess (queueB, queueA, p);
                             p->quantum = quantumA;
                         } else {
                             priorityEnqueueTask(readyQueueB, t);
                         }

                         t = getNextTaskPreemptive(queueB, readyQueueB, stats->runtime);
                     } else {
                        switch (t->type) {
                            case 'i':
                                if (p->quantum > 0) { // if process has quantum left
                                    p->quantum--;
                                    if (p->quantum > 0) {
                                        p->completions++;
                                        if (p->completions == 3) { // promote to queue A
                                            p->quantum = quantumA;
                                            priorityEnqueueProcess(queueA, p);
                                            p->endQueue = "A";
                                        }
                                    } else { // reset completions
                                        p->completions = 0;
                                    }
                                    stats->instructions++;
                                    enqueueTask(ioQueue, t); // add to I/O queue
                                } else {
                                    t->interrupts++;
                                    p->taskRunning = 0;
                                    if (t->interrupts == 3) { // promote to queue A
                                        p->quantum = quantumA;
                                        // p->taskRunning = 0;
                                        promoteProcess(queueB, queueA, p);
                                        priorityEnqueueTask(readyQueueA, t);
                                    } else { // reset completions
                                        p->quantum = quantumB;
                                        p->completions = 0;
                                        // p->taskRunning = 0;
                                        priorityEnqueueTask(readyQueueB, t);
                                    }
                                }
                                cpu = 0;
                                break;
                            case 'e':
                                if (t->time == 0) { // task completed
                                    t->completed = 1;
                                    p->taskRunning = 0;
                                    p->currentTask++;
                                    stats->instructions++;
                                    if (p->quantum > 0) { // if quantum not used up
                                        p->completions++;
                                        if (p->completions == 3) { // promote to queue A
                                            p->quantum = quantumA;
                                            promoteProcess(queueB, queueA, p);
                                        }
                                    } else { // reset completions
                                        p->completions = 0;
                                    }
                                    cpu = 0;
                                } else if (p->quantum == 0) { // quantum used up
                                    p->completions = 0;
                                    t->interrupts++;
                                    p->taskRunning = 0;
                                    if (t->interrupts == 3) { // promote to queue A
                                        p->quantum = quantumA;
                                        promoteProcess(queueB, queueA, p);
                                        priorityEnqueueTask(readyQueueA, t);
                                        cpu = 0;
                                    } else { // put back in ready queue
                                        p->quantum = quantumB;
                                        priorityEnqueueTask(readyQueueB, t);
                                        cpu = 0;
                                    }
                                    cpu = 0;
                                } else {
                                    t->time--;
                                    p->quantum--;
                                }
                                break;
                            default: // 't' - terminate process
                                if (p->quantum > 0) {
                                    p->quantum--;
                                    stats->instructions++;
                                    stats->runtime++;
                                    p->taskRunning = 0;
                                    p->runtime = stats->runtime;
                                    stats->minWait = stats->minWait < p->ready ? stats->minWait : p->ready;
                                    stats->maxWait = stats->maxWait > p->ready ? stats->maxWait : p->ready;
                                    stats->totalWait += p->ready;
                                    endProcess(queueB, exitQueue, p);
                                    cpu = 0;
                                } else {
                                    p->completions = 0;
                                    t->interrupts++;
                                    p->taskRunning = 0;
                                    p->quantum = quantumB;
                                    priorityEnqueueTask(readyQueueB, t);
                                    cpu = 0;
                                }
                                cpu = 0;
                                break;
                        }
                     }
                 }

                 if (!isEmptyP(queueB)) {
                     updateProcessQueue(queueB, stats->runtime);
                 }

                 stats->runtime++;
             }
         }
     }
     // free memory
     free(queueA);
     free(queueB);
     free(ioQueue);
     free(readyQueueA);
     free(readyQueueB);

     // print final stats
     printStats(exitQueue, stats);

     // Free exit queue
     free(exitQueue);
     free(stats);
 }

/*
 * Function: runNonPreemption
 *
 * Runs the simulation for non-preemption scheduling
 */
void runNonPreemption(int quantumA, int quantumB, pQueue *queueB) {

    // Initialize simulation variables
    Stats *stats = initializeStats();
    Task *t = NULL;
    Process *p = NULL;
    int cpu = 0;

    // Initialize queues
    pQueue *queueA = createProcessQueue();
    pQueue *exitQueue = createProcessQueue();
    tQueue *ioQueue = createTaskQueue();
    tQueue *readyQueueA = createTaskQueue();
    tQueue *readyQueueB = createTaskQueue();

    // simulation start time == first process arrival time
    p = peekProcess(queueB);
    stats->runtime = stats->startTime = p->arrival;
    p = NULL;

    // main simulation loop for non-preemption
    while (!allQueuesEmpty(queueA, queueB, readyQueueA, readyQueueB, ioQueue)) {

        // prioritize queue A
        if (!isEmptyP(queueA) || !isEmptyT(readyQueueA)) {

            while (!isEmptyP(queueA) || !isEmptyT(readyQueueA)) {

                // update I/O tasks to simulate concurrent execution
                if (!isEmptyT(ioQueue)) {
                    updateIOTasks(ioQueue);
                }

                if (cpu == 0) {
                    // fetch next task and set CPU flag
                    t = getNextTask(queueA, readyQueueA, stats->runtime);
                    if (t != NULL) {
                        cpu = 1;
                        t->parent->taskRunning = 1;
                    }

                    break;
                } else {
                    p = t->parent;
                    switch (t->type) {
                        case 'i':
                            if (p->quantum > 0) { // if process has quantum left
                                p->quantum--;
                                if (p->quantum > 0) {
                                    p->completions++;
                                    // p->taskRunning = 0;
                                } else { // reset completions
                                    p->completions = 0;
                                }
                                stats->instructions++;
                                enqueueTask(ioQueue, t); // add to I/O queue
                            } else {
                                t->interrupts++;
                                p->taskRunning = 0;
                                p->quantum = quantumA;
                                priorityEnqueueTask(readyQueueA, t);
                            }
                            cpu = 0;
                            break;
                        case 'e':
                            if (t->time == 0) { // task completed
                                t->completed = 1;
                                p->taskRunning = 0;
                                p->currentTask++;
                                stats->instructions++;
                                cpu = 0;
                            } else if (p->quantum <= 0) { // quantum used up
                                p->completions = 0;
                                t->interrupts++;
                                p->taskRunning = 0;
                                p->quantum = quantumA;
                                priorityEnqueueTask(readyQueueA, t);
                                cpu = 0;
                            } else {
                                t->time--;
                                p->quantum--;
                            }
                            break;
                        default: // 't' - terminate process
                            if (p->quantum > 0) {
                                p->quantum--;
                                stats->instructions++;
                                stats->runtime++;
                                p->taskRunning = 0;
                                p->runtime = stats->runtime;
                                stats->minWait = stats->minWait < p->ready ? stats->minWait : p->ready;
                                stats->maxWait = stats->maxWait > p->ready ? stats->maxWait : p->ready;
                                stats->totalWait += p->ready;
                                endProcess(queueA, exitQueue, p);
                            } else {
                                p->completions = 0;
                                t->interrupts++;
                                p->taskRunning = 0;
                                p->quantum = quantumA;
                                priorityEnqueueTask(readyQueueA, t);
                            }
                            cpu = 0;
                            break;
                    }
                }

                // update queueA wait/ready times
                if (!isEmptyP(queueA)) {
                    updateProcessQueue(queueA, stats->runtime);
                }

                // update queueB wait/ready times
                if (!isEmptyP(queueB)) {
                    updateProcessQueue(queueB, stats->runtime);
                }

                stats->runtime++;
            }
        } else { // if queue A is empty

            while (!isEmptyP(queueB) || !isEmptyT(ioQueue) || !isEmptyT(readyQueueB)) {

                // update I/O queue to simulate concurrent execution
                if (!isEmptyT(ioQueue)) {
                    updateIOTasks(ioQueue);
                }

                if (cpu == 0) {
                    // fetch next task and set CPU flag
                    t = getNextTask(queueB, readyQueueB, stats->runtime);
                    if (t != NULL) {
                        cpu = 1;
                        t->parent->taskRunning = 1;
                    }

                    break;
                } else {
                    p = t->parent; // identify parent process

                    switch (t->type) {
                        case 'i':
                            if (p->quantum > 0) { // if process has quantum left
                                p->quantum--;
                                if (p->quantum > 0) {
                                    p->completions++;
                                    if (p->completions == 3) { // promote to queue A
                                        p->quantum = quantumA;
                                        priorityEnqueueProcess(queueA, p);
                                        p->endQueue = "A";
                                    }
                                } else { // reset completions
                                    p->completions = 0;
                                }
                                stats->instructions++;
                                enqueueTask(ioQueue, t); // add to I/O queue
                            } else {
                                t->interrupts++;
                                p->taskRunning = 0;
                                if (t->interrupts == 3) { // promote to queue A
                                    p->quantum = quantumA;
                                    // p->taskRunning = 0;
                                    promoteProcess(queueB, queueA, p);
                                    priorityEnqueueTask(readyQueueA, t);
                                } else { // reset completions
                                    p->quantum = quantumB;
                                    p->completions = 0;
                                    // p->taskRunning = 0;
                                    priorityEnqueueTask(readyQueueB, t);
                                }
                            }
                            cpu = 0;
                            break;
                        case 'e':
                            if (t->time == 0) { // task completed
                                t->completed = 1;
                                p->taskRunning = 0;
                                p->currentTask++;
                                stats->instructions++;
                                if (p->quantum > 0) { // if quantum not used up
                                    p->completions++;
                                    if (p->completions == 3) { // promote to queue A
                                        p->quantum = quantumA;
                                        promoteProcess(queueB, queueA, p);
                                    }
                                } else { // reset completions
                                    p->completions = 0;
                                }
                                cpu = 0;
                            } else if (p->quantum == 0) { // quantum used up
                                p->completions = 0;
                                t->interrupts++;
                                p->taskRunning = 0;
                                if (t->interrupts == 3) { // promote to queue A
                                    p->quantum = quantumA;
                                    promoteProcess(queueB, queueA, p);
                                    priorityEnqueueTask(readyQueueA, t);
                                    cpu = 0;
                                } else { // put back in ready queue
                                    p->quantum = quantumB;
                                    priorityEnqueueTask(readyQueueB, t);
                                    cpu = 0;
                                }
                                cpu = 0;
                            } else {
                                t->time--;
                                p->quantum--;
                            }
                            break;
                        default: // 't' - terminate process
                            if (p->quantum > 0) {
                                p->quantum--;
                                stats->instructions++;
                                stats->runtime++;
                                p->taskRunning = 0;
                                p->runtime = stats->runtime;
                                stats->minWait = stats->minWait < p->ready ? stats->minWait : p->ready;
                                stats->maxWait = stats->maxWait > p->ready ? stats->maxWait : p->ready;
                                stats->totalWait += p->ready;
                                endProcess(queueB, exitQueue, p);
                                cpu = 0;
                            } else {
                                p->completions = 0;
                                t->interrupts++;
                                p->taskRunning = 0;
                                p->quantum = quantumB;
                                priorityEnqueueTask(readyQueueB, t);
                                cpu = 0;
                            }
                            cpu = 0;
                            break;
                    }
                }

                if (!isEmptyP(queueB)) {
                    updateProcessQueue(queueB, stats->runtime);
                }

                stats->runtime++;
            }
        }
    }
    // free memory
    free(queueA);
    free(queueB);
    free(ioQueue);
    free(readyQueueA);
    free(readyQueueB);

    // print final stats
    printStats(exitQueue, stats);

    // Free exit queue
    free(exitQueue);
    free(stats);
}

/*
 * Function: printStats
 *
 * Prints the final statistics of the simulation
 */
void printStats(pQueue *exitQueue, Stats *stats) {

    printf("Start/End Time: %d, %d\n", stats->startTime, stats->runtime);
    printf("Processes completed: %d\n", exitQueue->size);
    printf("Instructions completed: %d\n", stats->instructions);
    printf("Average ready time: %.2f\n", stats->totalWait / exitQueue->size);
    printf("Max ready time: %d\n", stats->maxWait);
    printf("Min ready time: %d\n", stats->minWait);
    while (!isEmptyP(exitQueue)) {
        Process *p = dequeueProcess(exitQueue);
        printf("P%d time_completion:%d time_waiting:%d termination_queue:%s\n", p->pid, p->runtime, p->ready, p->endQueue);

        // free the process
        free(p->tasks);
        free(p);
    }
}

/*
 * Function: main
 *
 * Usage: ./a.out <input-file> <quantumA> <quantumB> <preemption>
 */
int main(int argc, char *argv[]) {

    // check for correct number of arguments
    if (argc != 5) {
        printf("\nIncorrect num of arguments\n");
        printf("Usage: %s <input-file> <quantumA> <quantumB> <preemption>\n\n", argv[0]);
        return 1;
    }

    // check for valid quantum values
    if (atoi(argv[2]) < 2 || atoi(argv[3]) < 2) {
        printf("\nInvalid arguments: quantumA and quantumB must be greater than 1\n");
        printf("Usage: %s <input-file> <quantumA> <quantumB> <preemption>\n\n", argv[0]);
        return 1;
    }

    // initialize the simulation struct
    Simulation sim;

    // Assign stats
    sim.quantumA = atoi(argv[2]);
    sim.quantumB = atoi(argv[3]);
    sim.preemption = atoi(argv[4]);
    sim.start = 0;
    sim.end = 0;

    // Open the input file
    sim.input_file = fopen(argv[1], "r");
    if (sim.input_file == NULL) {
        if (errno == ENOENT) {
            printf("Error: File %s does not exist\n", argv[1]);
        } else {
            printf("Error: Could not open file %s\n", argv[1]);
        }
        return 1;
    }

    // Parse the input file
    pQueue *queueB = ParseFile(sim.input_file, sim.quantumB);

    // Close the input file
    fclose(sim.input_file);

    // Run simulation
    Simulate(sim.quantumA, sim.quantumB, sim.preemption, queueB);
}
