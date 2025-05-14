/*
 * queue.h
 *
 * Author: Andrew Cox
 * Date: 03 May 2024
 *
 * This file contains the definition of a queue data structure.
 */

 #ifndef QUEUE_H
 #define QUEUE_H

 // forward declaration of structs
 struct Task;
 struct Process;
 struct Stats;
 struct tQueue;
 struct pQueue;
 struct tNode;
 struct pNode;

 // Struct for task node
 typedef struct tNode {
     struct Task *task;         // pointer to a task object
     struct tNode *next;        // pointer to the next node in the queue
 } tNode;

 // Struct for process node
 typedef struct pNode {
     struct Process *process;   // pointer to a process object
     struct pNode *next;        // pointer to the next node in the queue
 } pNode;

 // Struct for task queue
 typedef struct tQueue {
     tNode *head;               // pointer to the first node in the queue
     tNode *tail;               // pointer to the last node in the queue
     int size;                  // number of nodes in the queue
 } tQueue;

 // Struct for process queue
 typedef struct pQueue {
     pNode *head;               // pointer to the first node in the queue
     pNode *tail;               // pointer to the last node in the queue
     int size;                  // number of nodes in the queue
 } pQueue;

 // Struct for task object
 typedef struct Task {
     char type;                 // 'e' = execution, 'i' = I/O, 't' = terminate
     int time;                  // time to execute or I/O time
     int wait;                  // ready/wait time --- not used but will seg fault if removed
     int completed;             // 0 = not completed, 1 = completed
     int interrupts;            // number of interrupts
     struct Process *parent;    // pointer to parent process
 } Task;

 // Struct for process object
 typedef struct Process {
     int pid;                   // process id
     int priority;              // process priority
     int arrival;               // arrival time
     int runtime;               // total runtime

     tQueue *tasks;             // queue of tasks
     int numTasks;              // number of tasks
     int currentTask;           // index of current task
     int completions;           // number of tasks completed under quantum

     int interrupts;            // number of interrupts
     int ready;                 // time process is ready/waiting to execute
     int taskRunning;           // flag to indicate a task is running
     int quantum;               // quantum time for execution tasks
     int bursts;                // number of bursts for execution tasks
     char *endQueue;            // final queue for process
 } Process;


 /**************************************************************************
  * Function Prototypes -- TASKS
  **************************************************************************/
 Task *createTask();
 void appendTask(Process *p, Task newTask); // deprecated
 void endTask(Task *t, struct Stats *s); // deprecated
 tQueue *createTaskQueue();
 void enqueueTask(tQueue *q, Task *t);
 void frontloadTask(tQueue *q, Task *t);
 void priorityEnqueueTask(tQueue *q, Task *t);
 Task *dequeueTask(tQueue *q);
 void removeTask(tQueue *q, Task *t);
 void *peekTask(tQueue *q);
 Task *getNextTask(pQueue *q, tQueue *ready, int runtime);
 int preemptionCheck (pQueue *q, tQueue *ready, Task *t, int runtime);
 Task *getNextTaskPreemptive (pQueue *q, tQueue *ready, int runtime);
 void updateIOTasks(tQueue *q);
 int isEmptyT(tQueue *q);

 /**************************************************************************
  * Function Prototypes -- PROCESSES
  **************************************************************************/
 Process *createProcess();
 pQueue *createProcessQueue();
 void enqueueProcess(pQueue *q, Process *p);
 void frontloadProcess(pQueue *q, Process *p);
 void priorityEnqueueProcess(pQueue *q, Process *p);
 Process *dequeueProcess(pQueue *q);
 void promoteProcess(pQueue *queueB, pQueue *queueA, Process *p);
 void endProcess(pQueue *q, pQueue *exit, Process *p);
 void updateProcessQueue(pQueue *q, int runtime);
 void *peekProcess(pQueue *q);
 int isEmptyP(pQueue *q);


 #endif
