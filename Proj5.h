//
//  Proj5.h
//  Proj5
//
//  Created by Tony on 11/2/16.
//  Copyright © 2016 Anthony Cavallo. All rights reserved.
//

#ifndef Proj5_h
#define Proj5_h

#include <stdio.h>          /* Basic functions */
#include <stdlib.h>         /* Some other basic functions */
#include <unistd.h>         /* For execl */
#include <signal.h>         /* For signal handling */
#include <getopt.h>         /* For command line options */
#include <string.h>         /* atoi(string) */
#include <ctype.h>          /* For isprint in getopt */
#include <time.h>           /* srand(unsigned) */
#include <sys/time.h>       /* POSIX standard, same as time.h */
#include <sys/shm.h>        /* For shared memory */
#include <sys/msg.h>        /* For message passing */
#include <semaphore.h>      /* For semaphore usage */
#include <stdbool.h>
//#include <sys/sem.h>
#include <sys/ipc.h>        /* For inter-process communication */
//#include <sys/types.h>      /*  */
#include <errno.h>          /* To set errno for perror */
#include <fcntl.h>          /* For O_* constants */
#include <sys/stat.h>       /* For mode constants */

#define MEMORY_KEY 18137644
#define RESOURCE_KEY 44673181
#define BILLION 1000000000L

/* Struct for resources */
typedef struct resource_control_block {
    bool isShareable;               /* Will either equal 1 or 0 */
    int maxResourceCount;           /* The total number of resources for requesting purposes */
    int currentResourceCount;       /* The current number of resources */
    int resourcesAllocated[18];     /* Shows where resources are allocated */
} rcb_t;

rcb_t *RCB_array;

/* Global variables for all */
int memory_size = sizeof(long long unsigned) * 3;
int time_memory, resource_memory;
long long unsigned *seconds;
long long unsigned *nano_seconds;

sem_t *sem;


/* Global variables for OSS */
int throughput;
int turnaround_time;
int waiting_time;
int verbose;
double cpu_utilization;

/* Global variables for USER */


/* Function prototypes for all */
void detachMemory();
void alarmHandler();
void segfaultHandler();
void interruptHandler();

/* Function prototypes for OSS */
void printHelpMenu();
void deadlockDetection();
void setupResourceBlocks();

#endif /* Proj5_h */
