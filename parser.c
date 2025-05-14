/*
 * parser.c
 *
 * Author: Andrew Cox
 * Date: 03 May 2024
 *
 * The file contains the implementation of the file parser for Simulation.c
 * The parser reads a file and creates process and task objects from the file
 * The parser returns the initial process queue for the simulation
 */

#include <stdlib.h>

#include "parser.h"


// file parser function
pQueue *ParseFile(FILE* file, int quantumB) {

    // create process and task pointers
    Process *p = NULL;
    Task *t = NULL;

    // create a character to read from file
    char c;

    // create process queue
    pQueue *q = createProcessQueue();

    // read file until end of file
    while ((c = fgetc(file)) != EOF) {

        ungetc(c, file);

        switch(c) {
            case 'P':
                // create process
                p = createProcess();
                p->quantum = quantumB;

                // assign process pid and priority
                if (fscanf(file, "P%d:%d", &(p->pid), &(p->priority)) != 2) {
                    fprintf(stderr, "Error reading process\n");
                    exit(EXIT_FAILURE);
                }

                break;
            case 'a':
                // assign arrival time
                if (fscanf(file, "arrival_t:%d", &(p->arrival)) != 1) {
                    fprintf(stderr, "Error reading arrival time\n");
                    exit(EXIT_FAILURE);
                }

                break;
            case 'i':
                // create io task
                t = createTask();
                t->type = 'i';

                if (fscanf(file, "io:%d", &(t->time)) != 1) {
                    fprintf(stderr, "Error reading io time\n");
                    exit(EXIT_FAILURE);
                }

                // assign parent process
                t->parent = p;

                // add task to process
                if (t != NULL) {
                    enqueueTask(p->tasks, t);
                    t = NULL;
                }

                break;
            case 'e':
                // create exe task
                t = createTask();
                t->type = 'e';

                // assign exe time
                if (fscanf(file, "exe:%d", &(t->time)) != 1) {
                    fprintf(stderr, "Error reading exe time\n");
                    exit(EXIT_FAILURE);
                }

                // assign parent process
                t->parent = p;

                // add task to process
                if (t != NULL) {
                    enqueueTask(p->tasks, t);
                    t = NULL;
                }

                break;
            case 't':
                // create terminate task
                t = createTask();
                t->type = 't';
                t->parent = p;

                // add task to process
                if (t != NULL) {
                    enqueueTask(p->tasks, t);
                    t = NULL;
                }

                // add process to queue B
                if (p != NULL) {
                    enqueueProcess(q, p);
                    p->endQueue = "B";
                    p = NULL; // reset process
                }

                // consume the rest of the line
                while ((c = fgetc(file)) != '\n' && !(feof(file)));

                continue;
        }

        // move to the next line
        while ((c = fgetc(file)) != '\n' && c != EOF);
    }

    // ensure uncaught processes are added to the queue
    if (p != NULL) {
        enqueueProcess(q, p);
        p->endQueue = "B";
        p = NULL;
    }

    return q;
}
