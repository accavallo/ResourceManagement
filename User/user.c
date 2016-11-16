//
//  main.c
//  User
//
//  Created by Tony on 11/2/16.
//  Copyright © 2016 Anthony Cavallo. All rights reserved.
//

#include "Proj5.h"

int main(int argc, const char * argv[]) {
    signal(SIGINT, interruptHandler);
    signal(SIGSEGV, segfaultHandler);
    signal(SIGALRM, alarmHandler);
    
    /* Need another interrupt handler, maybe, for when deadlock detection terminates a process */
    
    /***********
     argv[0] is the process ID as given by oss.
     argv[1] is the creation time of the process.
     argv[2] is the logical run time for oss.
     ***********/
    
    /* Get shared memory for time */
    if((time_memory = shmget(MEMORY_KEY, memory_size, 0777)) == -1) {
        printf("User %i (process %i) failed to get shared memory. Exiting program...\n", atoi(argv[0]), getpid());
        perror("User shmget time memory:");
        exit(1);
    }
    
    /* Attach to shared memory */
    if ((seconds = shmat(time_memory, NULL, 0)) == (long long unsigned *)-1) {
        printf("User %i (process %i) failed to attach to shared memory. Exiting program...\n", atoi(argv[0]), getpid());
        perror("User shmat time memory:");
        exit(1);
    }
    nano_seconds = seconds + 1;
    
    /* Get shared memory for the reource control blocks */
    if((resource_memory = shmget(RESOURCE_KEY, sizeof(rcb_t) * 20, 0777)) == -1) {
        printf("\n");
        perror("");
        exit(1);
    }
    /* Attach to the resource control blocks */
    if ((RCB_array = shmat(resource_memory, NULL, 0)) == NULL) {
        printf("\n");
        perror("");
        exit(1);
    }
    
    /* Open the named semaphore since sem_init() is deprecated... */
    sem = sem_open("/mySem", 0);
    if (sem == SEM_FAILED) {
        perror("User sem_open");
        printf("User %i failed to open semaphore. Exiting program...\n", atoi(argv[0]));
        exit(1);
    }

    printf("Hello, from process %3i at time %.09f!\n", atoi(argv[0]), *seconds + (double)*nano_seconds / BILLION);

    sem_wait(sem);
    
    const double CREATION_TIME = atof(argv[1]);
    double current_time = *seconds + (double)*nano_seconds / BILLION;
    int continueLoop = 1;
    /* In this loop there needs to be some resource requests. */
    //TODO: Determine number of resources to request.
    //TODO: Make resource requests at random times for random resources.
    while (continueLoop) {
        //TODO: Check between 1 ms and 250 ms to determine if the process will finish.
        if (current_time - CREATION_TIME >= 1) {
            printf("Process %s has been around for %.09f seconds.\n", argv[0], current_time - CREATION_TIME);
            continueLoop = 0;
        } else if (atoi(argv[2]) <= *seconds)
            continueLoop = 0;
        
        /* Update the current time */
        current_time = *seconds + (double)*nano_seconds / BILLION;
    }
    
    sem_post(sem);
    printf("Process %i ending at time %.09f.\n", atoi(argv[0]), *seconds + (double)*nano_seconds / BILLION);
    
    detachMemory();
    return 0;
}

void detachMemory() {
    shmdt(RCB_array);
    
    shmdt(seconds);
    shmdt(nano_seconds);
    exit(0);
}

void segfaultHandler() {    
    fprintf(stderr, "USER process %i suffered a segmentation fault. It's still dead Jim....\n", getpid());
    detachMemory();
}

void interruptHandler() {
    fprintf(stderr, "USER process %i got interrupted while being played with.\n", getpid());
    detachMemory();
}

void alarmHandler() {
    printf("User process %i is being put away because time is up.\n", getpid());
    detachMemory();
}
