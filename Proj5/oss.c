//
//  main.c
//  Proj5
//
//  Created by Tony on 11/2/16.
//  Copyright © 2016 Anthony Cavallo. All rights reserved.
//

#include "Proj5.h"
static long long unsigned next_creation_time = 0;

int main(int argc, const char * argv[]) {
    signal(SIGSEGV, segfaultHandler);
    signal(SIGINT, interruptHandler);
    printf("This is OSS son!\n");
    verbose = 0;
    int option, index;
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
    
    for (index = optind; index < argc; index++)
        printf("Non-option argument \"%s\"\n", argv[index]);
    
    printf("Logfile: %s\n", logfile);
    if (verbose)
        printf("Verbose logfile entries are on.\n");
    
    
    
    /* Create shared memory */
    if((time_memory = shmget(MEMORY_KEY, memory_size, IPC_CREAT | 0777)) == -1) {
        printf("OSS failed to create shared memory. Exiting program...\n");
        perror("OSS shmget time memory:");
        exit(1);
    }
    
    /* Attach to shared memory */
    if((seconds = shmat(time_memory, NULL, 0)) == (long long unsigned *)-1) {
        printf("OSS failed to attach to shared memory. Exiting program...\n");
        perror("OSS shmat time memory:");
        exit(1);
    }
    nano_seconds = seconds + 1;
    *seconds = 0;
    *nano_seconds = 0;
    
    /* Create shared memory for the resource control blocks */
    if((resource_memory = shmget(RESOURCE_KEY, sizeof(rcb_t) * 20,  IPC_CREAT | 0777)) == -1) {
        printf("OSS failed to create shared memory for the resource control blocks. Exiting program...\n");
        perror("OSS shmget rcb:");
        exit(1);
    }
    
    
    if ((RCB_array = shmat(resource_memory, NULL, 0)) == NULL) {
        printf("OSS failed to attach to resource memory. Exiting program...\n");
        perror("OSS shmat resource memory:");
        exit(1);
    }
    
    /* Set up initial resources for the resource control blocks */
    setupResourceBlocks();
    
    /* Create a named semaphore */
    sem = sem_open("/mySem", O_CREAT, 0777, 1);
    /* Check if the semaphore created correctly */
    if(sem == SEM_FAILED) {
        printf("Semaphore failed to create. Exiting program...\n");
        perror("parent sem_open");
        exit(1);
    }
    /* Check if the sem_post succeeded */
    if (sem_post(sem) == -1)
        perror("oss sem_post");
    
    int max_processes = 18, process_count = 0, process_number = 1;
    char procID[10], process_creation_time[15], logical_run_time[3];
    pid_t wpid, pid;
    int status, runtime = 5;
    srand((unsigned)time(NULL));
    printf("And now to continue with your regularly scheduled program\n");
    alarm(30);
    signal(SIGALRM, alarmHandler);
    while (*seconds < runtime) {
        *nano_seconds += rand() % 10000;
        if (*nano_seconds >= BILLION) {
            (*seconds)++;
            *nano_seconds %= BILLION;
            /* Check for deadlock and take corrective action if necessary. */
            deadlockDetection();
        }
        
        long long unsigned current_time = *seconds * BILLION + *nano_seconds;
        
        if (process_count < max_processes && next_creation_time <= current_time) {
            next_creation_time = (*seconds * BILLION + *nano_seconds + (1 + rand() % 500000000));
            
            process_count++;
            sprintf(procID, "%i", process_number);
            sprintf(process_creation_time, "%.09f", (double)current_time / BILLION);
            sprintf(logical_run_time, "%i", runtime);
            process_number++;
            pid = fork();
            if (pid == 0) {
                execl("./user", procID, process_creation_time, logical_run_time, (char *)NULL);
                printf("Process didn't exec properly.\n");
                exit(EXIT_FAILURE);
            }
        }
        
        int processID = waitpid(0, NULL, WNOHANG);
        if (processID > 0) {
            process_count--;
        }
    }
    
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
    printf("Time on deck: %.09f seconds.\n", *seconds + (double)*nano_seconds / BILLION);
    
    /* Unlink and close the semaphore. Unlink ONLY happens here, not in child processes. */
    sem_unlink("/mySem");
    sem_close(sem);
    
    /* Detach and release the memory for the resource blocks */
    shmdt(RCB_array);
    shmctl(resource_memory, IPC_RMID, NULL);
    
    /* Detach and release the memory for time */
    shmdt(nano_seconds);
    shmdt(seconds);
    shmctl(time_memory, IPC_RMID, NULL);
}

void alarmHandler() {
    printf("Time has expired!\n");
    pid_t groupID = getpgrp();
//    int status;
    killpg(groupID, SIGALRM);
    
//    while ((wpid = wait(&status)) > 0);
    
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

/*  */
void deadlockDetection() {
    printf("Running deadlock detection...\n");
    sleep(2);
    if (verbose) {
        printf("Writing more stuff to file because verbose statements are turned on.\n");
    }
    printf("Deadlock detection finished.\n");
}

/* Set up the resources blocks. */
void setupResourceBlocks() {
    printf("Setting up resource blocks...\n");
    srand((unsigned)time(NULL));
    int i = 0, willBeShareable = 15 + rand() % 11;
    printf("willBeShareable is set to %i\n", willBeShareable);
    for (; i < 20; i++) {
        if (rand() % 100 <= willBeShareable) {
            RCB_array[i].isShareable = 1;
            printf("Resource %i is shareable\n", i);
        } else {
            RCB_array[i].isShareable = 0;
        }
        
        /* What I have been told about shareable resources is that a request will always be granted. I'll have to do some additional setup in user so it won't request more than 10 if the resource is shareable. */
        if (RCB_array[i].isShareable) {
            RCB_array[i].maxResourceCount = 100;
        } else {
            RCB_array[i].maxResourceCount = 1 + rand() % 10;
        }
    }
    sleep(2);
}
