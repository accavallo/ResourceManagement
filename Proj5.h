//
//  Proj5.h
//  Proj5
//
//  Created by Tony on 11/2/16.
//  Copyright © 2016 Anthony Cavallo. All rights reserved.
//

#ifndef Proj5_h
#define Proj5_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <errno.h>

#define MEMORY_KEY 18137644

/* Global variables for all */
int time_memory;
long long unsigned *seconds;
long long unsigned *nano_seconds;

/* Global variables for OSS */
int throughput;
int turnaround_time;
int waiting_time;
double cpu_utilization;

/* Global variables for USER */


/* Function prototypes */
void detachMemory();
void alarmHandler();
void segfaultHandler();
void interruptHandler();

#endif /* Proj5_h */
