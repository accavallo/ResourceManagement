//
//  main.c
//  Proj5
//
//  Created by Tony on 11/2/16.
//  Copyright © 2016 Anthony Cavallo. All rights reserved.
//

#include "Proj5.h"

int main(int argc, const char * argv[]) {
    signal(SIGSEGV, segfaultHandler);
    signal(SIGINT, interruptHandler);
    printf("This is OSS son!\n");
    
    const int NANO = 1000000000L;
    
    /* Create shared memory */
    if((time_memory = shmget(MEMORY_KEY, sizeof(long long unsigned) * 3, IPC_CREAT | 0777)) == -1) {
        printf("OSS failed to create shared memory. Exiting program...\n");
        perror("OSS shmcreat");
        exit(1);
    }
    
    /* Attach to shared memory */
    if((seconds = shmat(time_memory, NULL, 0)) == (long long unsigned *)-1) {
        printf("OSS failed to attach to shared memory. Exiting program...\n");
        perror("OSS shmat");
        exit(1);
    }
    nano_seconds = seconds + 1;
    
    *seconds = 0;
    *nano_seconds = 0;
    
    srand((unsigned)time(NULL));
    alarm(20);
    signal(SIGALRM, alarmHandler);
    while (*seconds < 10) {
        *nano_seconds += rand() % 10000;
        if (*nano_seconds > NANO) {
            (*seconds)++;
            *nano_seconds %= NANO;
        }
        
//        if (fork() == 0) {
//            execl("./user", (char *)NULL);
//            exit(EXIT_FAILURE);
//        }
    }
    
    
    /* Wait for all child processes to finish. */
    pid_t wpid;
    int status;
    while ((wpid = wait(&status)) > 0);
    
    printf("Time on deck: %.09f seconds.\n", *seconds + (double)*nano_seconds / NANO);
    detachMemory();
    return 0;
}


/* Detach from and release all shared memory */
void detachMemory() {
    shmdt(nano_seconds);
    shmdt(seconds);
    shmctl(time_memory, IPC_RMID, NULL);
}

void alarmHandler() {
    printf("Time has expired!\n");
    
    killpg(getpgrp(), SIGALRM);
    pid_t wpid; int status;
    while ((wpid = wait(&status)) > 0);
    
    detachMemory();
    exit(0);
}

void segfaultHandler() {
    printf("Good job, you created a segmentation fault. Are you proud of yourself? ARE YOU?!\n");
    
    /* Send the segmentation fault signal to all child processes. */
    killpg(getpgrp(), SIGSEGV);
    pid_t wpid; int status;
    while ((wpid = wait(&status)) > 0);
    
    fprintf(stderr, "OSS process %i suffered a segmentation fault. It's dead Jim....\n", getpid());
    detachMemory();
}

void interruptHandler() {
    printf("KNOCK ON MY DOOR, KNOCK NEXT TIME! Did you see anything?!\nNo, sir! I didn't see you playing with your processes again!\n");
    
    /* Send the interrupt signal to child processes. */
    killpg(getpgrp(), SIGINT);
    pid_t wpid; int status;
    while ((wpid = wait(&status)) > 0);
    
    fprintf(stderr, "OSS process %i got interrupted while playing with its processes\n", getpid());
    detachMemory();
}
