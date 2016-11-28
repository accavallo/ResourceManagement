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
#include <stdbool.h>        /* For bool type because C doesn't include it automatically. I really hate this language */
#include <sys/ipc.h>        /* For inter-process communication */
#include <sys/types.h>      /*  */
#include <errno.h>          /* To set errno for perror */
#include <fcntl.h>          /* For O_* constants */
#include <sys/stat.h>       /* For mode constants */

#define MEMORY_KEY 18137644
#define RESOURCE_KEY 44673181
#define QUEUE_KEY 18441376
#define STATS_KEY 163216
#define BILLION 1000000000L

typedef struct resource_control_block {
    bool isShareable;               /* Will either be true or false */
    int maxResourceCount;           /* The total number of resources for requesting purposes */
    int currentResourceCount;       /* The current number of resources */
} rcb_t;
rcb_t *RCB_array;

/* Global variables for all */
int memory_size = sizeof(long long unsigned) * 22;
int time_memory, resource_memory, queue_memory, stats_memory, *resourceVector, *resourceQueue;
long long unsigned *seconds, *nano_seconds, *claim;

typedef struct statsStruct {
    double cpu_utilization, waiting_time, turnaround_time;
}stats_t;
stats_t *myStats;

sem_t *resource_sem;


/* Global variables for OSS */
bool verbose;


/* Global variables for USER */
//resources[i][0] will be the number of resources currently claimed. resources[i][1] will be the number of resources desired to be claimed.
int resources[20][2] = {0}; /* Used to keep track of this process' resource claims */
int processID;

/* Function prototypes for all */
void detachMemory();
void alarmHandler();
void segfaultHandler();
void interruptHandler();
void signalHandler(int);
void printQueue(int);
void setupSharedMemory();

/* Function prototypes for OSS */
void printHelpMenu();
void deadlockDetection();
void setupResourceBlocks();

/* Function prototypes for USER */
void addToQueue(int);
void deleteFromQueue(int);

#endif /* Proj5_h */
