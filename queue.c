/*
 * Queue.c
 *
 * Author: Andrew Cox
 * Date: 03 May 2024
 *
 * This file contains the implementation of the Queue data structures.
 */

#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

/************************************************************
 * Task & Task Queue Functions
 ************************************************************/

 /*
  * Function: createTaskQueue
  *
  * Creates a new queue to hold tasks
  */
tQueue *createTaskQueue() {
    tQueue *q = (tQueue *)malloc(sizeof(tQueue));
    if (!q) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

/*
 * Function: createTask
 *
 * Creates a new task object
 */
Task *createTask() {
    Task *t = (Task *)malloc(sizeof(Task));
    if (!t) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    t->type = 'x';          // task type (io, exe, terminate)
    t->time = 0;            // time required for task
    t->completed = 0;       // flag for task completion
    t->interrupts = 0;      // number of times task was interrupted
    t->parent = NULL;       // pointer to parent process

    return t;
}

/*
 * Function: enqueueTask
 *
 * Adds a task to the end of the queue
 */
void enqueueTask(tQueue *q, Task *t) {
    tNode *newNode = (tNode *)malloc(sizeof(tNode));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newNode->task = t;
    newNode->next = NULL;

    if (q->head == NULL) {
        q->head = newNode;
        q->tail = newNode;
    } else {
        q->tail->next = newNode;
        q->tail = newNode;
    }

    q->size++;
}

/*
 * Function: frontloadTask
 *
 * Adds a task to the front of the queue
 */
void frontloadTask(tQueue *q, Task *t) {
    tNode *newNode = (tNode *)malloc(sizeof(tNode));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newNode->task = t;
    newNode->next = q->head;
    q->head = newNode;

    if (q->tail == NULL) {
        q->tail = newNode;
    }

    q->size++;
}

/*
 * Function: priorityEnqueueTask
 *
 * Adds a task to the queue based on the priority of the parent process
 */
void priorityEnqueueTask(tQueue *q, Task *t) {
    tNode *newNode = (tNode *)malloc(sizeof(tNode));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newNode->task = t;
    newNode->next = NULL;

    // if the queue is empty, add the task to the head
    if (q->head == NULL) {
        q->head = newNode;
        q->tail = newNode;
    } else {
        tNode *current = q->head;
        tNode *prev = NULL;
        // otherwise, add the task based on the priority of the parent process
        while (current != NULL && current->task->parent->priority > t->parent->priority) {
            prev = current;
            current = current->next;
        }

        if (prev == NULL) {
            newNode->next = q->head;
            q->head = newNode;
        } else if (current == NULL) {
            q->tail->next = newNode;
            q->tail = newNode;
        } else {
            prev->next = newNode;
            newNode->next = current;
        }
    }

    q->size++;
}

/*
 * Function: dequeueTask
 *
 * Removes the first task from the queue
 */
Task *dequeueTask(tQueue *q) {
    if (q->head == NULL) {
        return NULL;
    }

    tNode *temp = q->head;
    Task *t = temp->task;

    q->head = q->head->next;
    free(temp);

    q->size -= 1;

    return t;
}

/*
 * Function: removeTask
 *
 * Removes a specific task from the queue
 *
 */
void removeTask(tQueue *q, Task *t) {
    if (q->head == NULL) {
        return;
    }

    tNode *current = q->head;
    tNode *prev = NULL;

    // Find the task in the queue
    while (current != NULL && current->task != t) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        return;
    }

    // Remove the task from the queue
    if (prev == NULL) {
        q->head = current->next;
    } else {
        prev->next = current->next;
    }

    if (current == q->tail) {
        q->tail = prev;
    }

    // Free the memory allocated for the task
    free(current);

    q->size -= 1;
}

/*
 * Function: peekTask
 *
 * Returns the first task in the queue without removing it
 */
void *peekTask(tQueue *q) {
    if (q->head == NULL) {
        return NULL;
    }

    return q->head->task;
}

/*
 * Function: updateIOTasks
 *
 * Updates the time remaining for each I/O task currently running
 * and removes any completed tasks from the queue
 */
void updateIOTasks(tQueue *q) {
    if (!q) return; // Safety check for null queue

    tNode *prev = NULL;
    for (tNode *current = q->head; current != NULL; ) {
        Task *t = current->task;
        // Decrement time if task not yet completed
        if (t->time > 0) {
            t->time--;
            prev = current;
            current = current->next; // Move to the next node
        } else { // Task is complete
            t->completed = 1;
            t->parent->taskRunning = 0;
            t->parent->currentTask++;

            // Detach the current node from the queue
            if (prev == NULL) {
                q->head = current->next; // Current node is the head
                if (q->head == NULL) { // If the queue is now empty
                    q->tail = NULL;
                }
            } else { // Current node is not the head
                prev->next = current->next;
                if (current->next == NULL) {
                    q->tail = prev;
                }
            }

            // Prepare to free the current node and move to the next
            tNode *remove = current;
            current = current->next; // Update node before freeing
            free(remove); // Free the node
        }
    }
}

/*
 * Function: getNextTask
 *
 * Returns the next task to be executed based on the current runtime
 * and whether a process is currently running
 */
Task *getNextTask(pQueue *q, tQueue *ready, int runtime) {
    // Check if there are any tasks in the ready queue
    if (!isEmptyT(ready)) {
        Task *nextTask = dequeueTask(ready);
        if (nextTask) { // If a task is found, return it
            nextTask->parent->taskRunning = 1;
            return nextTask;
        }
    }

    // Check the process queue if no task in ready queue
    pNode *current = q->head;
    while (current != NULL) {
        Process *p = current->process;
        // Ensure the process has arrived and has tasks to run
        if (p->arrival <= runtime && p->taskRunning == 0 && !isEmptyT(p->tasks)) {
            Task *t = dequeueTask(p->tasks);
            if (t) { // If a task is found, return it
                t->parent->taskRunning = 1; // Set the process to running

                return t;
            }
        }
        current = current->next;
    }

    // If no tasks found return NULL
    return NULL;
}

/*
 * Function: preemptionCheck
 *
 * Checks if a task should be preempted based on the current runtime
 * and the tasks in the process queue
 */
int preemptionCheck (pQueue *q, tQueue *ready, Task *t, int runtime) {
    pNode *current = q->head;
    while (current != NULL) {
        Process *p = current->process;
        // Ensure the process has arrived and has tasks to run
        if (p->arrival <= runtime && p->taskRunning == 0 && !isEmptyT(p->tasks)) {
            Task *nextTask = peekTask(ready);
            if (nextTask && nextTask->parent->priority > t->parent->priority) {
                return 1; // should preempt
            }
        }
        current = current->next;
    }
    return 0; // should not preempt
}

/*
 * Function: getNextTaskPreemptive
 *
 * Returns the next task to be executed based on the current runtime
 * and process priority
 */
Task *getNextTaskPreemptive(pQueue *q, tQueue *ready, int runtime) {
    // Check if there are any tasks in the ready queue
    if (!isEmptyT(ready)) {
        Task *currentTask = dequeueTask(ready);
        Task *nextTask = peekTask(ready);
        if (!nextTask) { // If no other tasks in the ready queue
            if (currentTask) { // return the current task
                currentTask->parent->taskRunning = 1;
                return currentTask;
            }
        } else if (currentTask->parent->priority < nextTask->parent->priority) {
            priorityEnqueueTask(ready, currentTask);
            currentTask = dequeueTask(ready);
            if (currentTask) { // If a task is found, return it
                currentTask->parent->taskRunning = 1;
                return currentTask;
            }
        }
    }

    // Check the process queue if no task in ready queue
    pNode *current = q->head;
    while (current != NULL) {
        Process *p = current->process;
        Process *nextProcess = (current->next != NULL) ? current->next->process : NULL;

        // Ensure the process has arrived and has tasks to run
        if (p->arrival <= runtime && p->taskRunning == 0 && !isEmptyT(p->tasks)) {
            if (nextProcess && nextProcess->arrival <= runtime && nextProcess->taskRunning == 0 && !isEmptyT(nextProcess->tasks)) {
                if (p->priority < nextProcess->priority) {
                    Task *t = dequeueTask(p->tasks);
                    if (t) {
                        t->parent->taskRunning = 1; // Set the process to running
                        return t;
                    }
                } else {
                    Task *t = dequeueTask(nextProcess->tasks);
                    if (t) {
                        t->parent->taskRunning = 1; // Set the process to running
                        return t;
                    }
                }
            } else {
                Task *t = dequeueTask(p->tasks);
                if (t) { // If a task is found, return it
                    t->parent->taskRunning = 1; // Set the process to running
                    return t;
                }
            }
        }
        current = current->next;
    }

    // If no tasks found, return NULL
    return NULL;
}

/*
 * Function: isEmptyT
 *
 * Returns 1 if the task queue is empty, 0 otherwise
 */
int isEmptyT(tQueue *q) {
    return q && q->head == NULL;
}

/************************************************************
 * Process & Process Queue Functions
 ************************************************************/

 /*
  * Function: createProcessQueue
  *
  * Creates a new queue to hold processes
  */
pQueue *createProcessQueue() {
    pQueue *q = (pQueue *)malloc(sizeof(pQueue));
    if (q == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

/*
 * Function: createProcess
 *
 * Creates a new process object
 */
Process *createProcess() {
    Process *p = (Process *)malloc(sizeof(Process));
    if (!p) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    p->pid = 0;                     // process ID
    p->priority = 0;                // process priority
    p->arrival = 0;                 // arrival time
    p->runtime = 0;                 // process runtime
    p->tasks = createTaskQueue();   // task queue
    p->numTasks = 0;                // number of tasks
    p->currentTask = 0;             // current task
    p->completions = 0;             // completions under quantum
    p->interrupts = 0;              // number of times interrupted
    p->ready = 0;                   // wait/ready time
    p->taskRunning = 0;             // flag for task running
    p->quantum = 0;                 // quantum time
    p->bursts = 0;                  // number of bursts
    p->endQueue = "B";              // final queue

    return p;
}

/*
 * Function: enqueueProcess
 *
 * Adds a process to the end of the queue
 */
void enqueueProcess(pQueue *q, Process *p) {
    pNode *newNode = (pNode *)malloc(sizeof(pNode));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newNode->process = p;
    newNode->next = NULL;

    if (q->head == NULL) {
        q->head = newNode;
        q->tail = newNode;
    } else {
        q->tail->next = newNode;
        q->tail = newNode;
    }

    q->size++;
}

/*
 * Function: frontloadProcess
 *
 * Adds a process to the front of the queue
 *
 * -- not currently used --
 */
void frontloadProcess(pQueue *q, Process *p) {
    pNode *newNode = (pNode *)malloc(sizeof(pNode));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Add the process to the front of the queue
    newNode->process = p;
    newNode->next = q->head;
    q->head = newNode;

    if (q->tail == NULL) {
        q->tail = newNode;
    }

    q->size++;
}

/*
 * Function: priorityEnqueueProcess
 *
 * Adds a process to the queue based on priority
 */
void priorityEnqueueProcess(pQueue *q, Process *p) {
    pNode *newNode = (pNode *)malloc(sizeof(pNode));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newNode->process = p;
    newNode->next = NULL;

    if (q->head == NULL) { // If the queue is empty
        q->head = newNode;
        q->tail = newNode;
    } else {
        pNode *current = q->head;
        pNode *prev = NULL;

        // Find the correct position to insert the new node
        while (current != NULL && current->process->priority > p->priority) {
            prev = current;
            current = current->next;
        }
        if (prev == NULL) {
            newNode->next = q->head;
            q->head = newNode;
        } else if (current == NULL) {
            q->tail->next = newNode;
            q->tail = newNode;
        } else {
            prev->next = newNode;
            newNode->next = current;
        }
    }

    q->size++;
}

/*
 * Function: dequeueProcess
 *
 * Removes a process from the front of the queue
 */
Process *dequeueProcess(pQueue *q) {
    if (q->head == NULL) {
        return NULL;
    }

    pNode *temp = q->head;
    Process *p = temp->process;
    q->head = q->head->next;
    free(temp);

    q->size -= 1;

    return p;
}

/*
 * Function: promoteProcess
 *
 * Removes a process from the queue and re-enqueues it based on priority
 */
 void promoteProcess(pQueue *queueB, pQueue *queueA, Process *p) {
    pNode *prev = NULL;
    pNode *current = queueB->head;

    // Locate the process in the current queue
    while (current != NULL) {
        if (current->process == p) {
            if (prev == NULL) { // Process is at the head of the queue
                queueB->head = current->next;
                if (queueB->head == NULL) { // Process is the only one in the queue
                    queueB->tail = NULL;
                }
            } else {
                prev->next = current->next;
                if (current->next == NULL) {
                    queueB->tail = prev; // Process is at the tail of the queue
                }
            }
            queueB->size--;

            // Re-enqueue the process based on priority
            priorityEnqueueProcess(queueA, p);
            p->endQueue = "A";
            free(current); // Free the node, but not the process

            return;
        }
        prev = current;
        current = current->next;
    }
    printf("PROMOTE PROCESS: Process not found in queue.\n");
 }

/*
 * Function: endProcess
 *
 * Removes a process from the queue and enqueues it to the exit queue
 */
void endProcess(pQueue *q, pQueue *exit, Process *p) {
    pNode *prev = NULL;
    pNode *current = q->head;

    // Locate the process in the current queue
    while (current != NULL) {
        if (current->process == p) {
            if (prev == NULL) { // Process is at the head of the queue
                q->head = current->next;
                if (q->head == NULL) { // Process is the only one in the queue
                    q->tail = NULL;
                }
            } else {
                prev->next = current->next;
                if (current->next == NULL) {
                    q->tail = prev; // Process is at the tail of the queue
                }
            }
            q->size--;

            // Enqueue the process to the exit queue
            enqueueProcess(exit, p);
            free(current); // Free the node, but not the process
            return;
        }
        // Move to the next node
        prev = current;
        current = current->next;
    }

    fprintf(stderr, "END PROCESS: Process not found in queue.\n");
    return;
}

/*
 * Function: peekProcess
 *
 * Returns the process at the front of the queue without removing it
 */
void *peekProcess(pQueue *q) {
    if (q->head == NULL) {
        return NULL;
    }

    return q->head->process;
}

/*
 * Function: updateProcessQueue
 *
 * Updates the wait/ready time for each process in the queue
 */
void updateProcessQueue(pQueue *q, int runtime) {
    pNode *current = q->head;
    while (current != NULL) {
        Process *p = current->process;
        // If the process is not running and has arrived, increment ready time
        if (p->taskRunning == 0 && p->arrival < runtime) {
            p->ready++;
        }
        current = current->next;
    }
}

/*
 * Function: isEmptyP
 *
 * Returns 1 if the queue is empty, 0 otherwise
 */
int isEmptyP(pQueue *q) {
    return q && q->head == NULL;
}
