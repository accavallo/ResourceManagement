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
    
    /* Get shared memory */
    if((time_memory = shmget(MEMORY_KEY, memory_size, 0777)) == -1) {
        printf("User %i (process %i) failed to get shared memory. Exiting program...\n", atoi(argv[0]), getpid());
        perror("User shmget: ");
        exit(1);
    }
    
    /* Attach to shared memory */
    if ((seconds = shmat(time_memory, NULL, 0)) == (long long unsigned *)-1) {
        printf("User %i (process %i) failed to attach to shared memory. Exiting program...\n", atoi(argv[0]), getpid());
        perror("User shmat: ");
        exit(1);
    }
    
    nano_seconds = seconds + 1;
    /* Open the named semaphore since sem_init() is deprecated... */
    sem = sem_open("/mySem", 0);
    if (sem == SEM_FAILED) {
        perror("user sem_open");
        printf("User %i failed to open semaphore. Exiting program...\n", atoi(argv[0]));
        exit(1);
    }

    printf("Hello, from process %3i at time %.09f!\n", atoi(argv[0]), *seconds + (double)*nano_seconds / NANO);

    sem_wait(sem);
    
    const double CREATION_TIME = atof(argv[1]);
    double current_time = *seconds + (double)*nano_seconds / NANO;
    int continueLoop = 1;
    while (continueLoop) {
        if (current_time - CREATION_TIME >= 1) {
            printf("Process %s has been around for %.09f seconds.\n", argv[0], current_time - CREATION_TIME);
            continueLoop = 0;
        } else if (atoi(argv[2]) <= *seconds)
            continueLoop = 0;
        
        current_time = *seconds + (double)*nano_seconds / NANO;
    }
    
    sem_post(sem);
    printf("Process %i ending at time %.09f.\n", atoi(argv[0]), *seconds + (double)*nano_seconds / NANO);
    
    detachMemory();
    return 0;
}

void detachMemory() {
    shmdt(seconds);
    shmdt(nano_seconds);
    exit(0);
}

void segfaultHandler() {
//    printf("Good job, you created a segmentation fault. Are you proud of yourself? ARE YOU?!\n");
    
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
