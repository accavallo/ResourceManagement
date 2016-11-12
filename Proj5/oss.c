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
    
    int option, index, verbose = 0;
    char *logfile = "logfile.txt";
    
    while ((option = getopt(argc, (char **)argv, "hvl:")) != -1) {
        switch (option) {
            case 'h':
                printHelpMenu();
                break;
            case 'v':
                verbose = 1;
                break;
            case 'l':
                logfile = optarg;
                break;
            case '?':
                if (optopt == 'l')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character '%s'.\n", optarg);
                break;
            default:
                break;
        }
    }
    
    for (index = optind; index < argc; index++) {
        printf("Non-option argument \"%s\"\n", argv[index]);
    }
    
    printf("Logfile: %s\n", logfile);
    if (verbose)
        printf("Verbose logfile entries are on.\n");
    
    
    
    /* Create shared memory */
    if((time_memory = shmget(MEMORY_KEY, memory_size, IPC_CREAT | 0777)) == -1) {
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
    
    
    int max_processes = 18, process_count = 0, process_number = 1;
    char procID[10], creationTime[15];
    pid_t wpid, pid;
    int status, runtime = 5;
    srand((unsigned)time(NULL));
    alarm(20);
    signal(SIGALRM, alarmHandler);
    while (*seconds < runtime) {
        *nano_seconds += rand() % 10000;
        if (*nano_seconds >= NANO) {
            (*seconds)++;
            *nano_seconds %= NANO;
        }
        
        long long unsigned current_time = *seconds * NANO + *nano_seconds;
        
        if (process_count < max_processes && current_time <= (runtime * NANO - (double)(runtime * NANO) * 0.1)) {
            pid = fork();
            process_count++;
            sprintf(procID, "%i", process_number);
            sprintf(creationTime, "%llu", current_time);
            process_number++;
            if (pid == 0) {
                execl("./user", procID, creationTime, (char *)NULL);
                exit(EXIT_FAILURE);
            }
        }
        
        int processID = waitpid(0, NULL, WNOHANG);
        if (processID > 0) {
            process_count--;
        }
        
    }
    
//    sleep(3);
    /* Wait for all child processes to finish. */
    while ((wpid = wait(&status)) > 0);
    
    sleep(1);
    printf("Finished waiting\n");
    
    detachMemory();
    return 0;
}

void printHelpMenu() {
    printf("'-h' prints this help menu.\n");
    printf("\"-l fileName\" will assign the argument 'fileName' to the variable 'logfile'.\n");
    printf("\n");
}


/* Detach from and release all shared memory */
void detachMemory() {
    printf("Time on deck: %.09f seconds.\n", *seconds + (double)*nano_seconds / NANO);
    
    shmdt(nano_seconds);
    shmdt(seconds);
    shmctl(time_memory, IPC_RMID, NULL);
}

void alarmHandler() {
    printf("Time has expired!\n");
    pid_t wpid, groupID = getpgrp();
    int status;
    killpg(groupID, SIGALRM);
    
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
