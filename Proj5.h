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
//#include <sys/sem.h>
#include <sys/ipc.h>        /* For inter-process communication */
#include <sys/types.h>      /*  */
#include <errno.h>          /* To set errno for perror */
#include <fcntl.h>          /* For O_* constants */
#include <sys/stat.h>       /* For mode constants */

#define MEMORY_KEY 18137644
#define RESOURCE_KEY 44673181
#define VECTOR_KEY 13761844
#define QUEUE_KEY 18441376
#define BILLION 1000000000L

/* Struct for resources */
typedef struct resourceQueue {
    struct resourceQueue *parent;
    struct resourceQueue *child;
    pid_t pid;
} rscq_t;

typedef struct resource_control_block {
    bool isShareable;               /* Will either be true or false */
    int maxResourceCount;           /* The total number of resources for requesting purposes */
    int currentResourceCount;       /* The current number of resources */
//    rscq_t *head;                   /* Shows where resources are allocated */
//    rscq_t *tail;                   /* Will be used for deadlock. The last resource in the queue will always be the one to go. */
} rcb_t;
rcb_t *RCB_array;

/* Global variables for all */
int memory_size = sizeof(long long unsigned) * 3;
int time_memory, resource_memory, vector_memory, queue_memory, *resourceVector, *resourceQueue;
long long unsigned *seconds, *nano_seconds;

sem_t *resource_sem;


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
void signalHandler(int);
void printQueue(int);

/* Function prototypes for OSS */
void printHelpMenu();
void setupSharedMemory();
void deadlockDetection();
void setupResourceBlocks();

/* Function prototypes for USER */
void addToQueue(int);
void deleteFromQueue(int);

#endif /* Proj5_h */
