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
    
    /***********
     argv[0] is the process ID as given by oss.
     argv[1] is the creation time of the process.
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
    
    printf("Hello, from process %i at time %.09f!\n", atoi(argv[0]), *seconds + (double)*nano_seconds / NANO);
    
    const int CREATION_TIME = atoi(argv[1]);
    while ((*seconds * NANO + *nano_seconds - CREATION_TIME) <= NANO) {
        
    }
//    printf("Process %i has now been around for at least 1 logical second.\n", atoi(argv[0]));
    sleep(1);
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
//    printf("KNOCK ON MY DOOR, KNOCK NEXT TIME! Did you see anything?!\nNo, sir! I didn't see you playing with your processes again!\n");
    
    
    fprintf(stderr, "USER process %i got interrupted while being played with.\n", getpid());
    detachMemory();
}

void alarmHandler() {
    printf("User process %i is being put away because time is up.\n", getpid());
    detachMemory();
}
