/*
 * parser.h
 *
 * Author: Andrew Cox
 * Date: 03 May 2024
 *
 * This file contains the function prototypes for the parser.c file.
 */

 #ifndef PARSER_H
 #define PARSER_H

 #include <stdio.h>
 //#include "process.h"
 #include "queue.h"

 // Function prototypes
 pQueue *ParseFile(FILE* file, int quantumB);

 #endif
