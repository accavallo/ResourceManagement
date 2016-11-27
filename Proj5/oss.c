//
//  main.c
//  Proj5
//
//  Created by Tony on 11/2/16.
//  Copyright © 2016 Anthony Cavallo. All rights reserved.
//

#include "Proj5.h"
static long long unsigned next_creation_time = 0;
static int jobs_completed = 0;
//int allocation[20];
char *logfile = "logfile.txt";

int main(int argc, const char * argv[]) {
    signal(SIGSEGV, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGALRM, signalHandler);
    signal(SIGTERM, signalHandler);
    verbose = false;
    int option, index, terminate_chance = 50, runtime = 25;
    
    while ((option = getopt(argc, (char **)argv, "hvl:t:r:")) != -1) {
        switch (option) {
            case 'h':
                printHelpMenu();
                break;
            case 'v':
                verbose = true;
                break;
            case 'l':
                logfile = optarg;
                break;
            case 't':
                if (atoi(optarg))
                    terminate_chance = atoi(optarg);
                else
                    printf("Exptected Integer, found %s instead", optarg);
                break;
            case 'r':
                if (atoi(optarg))
                    runtime = atoi(optarg);
                else
                    printf("Exptected Integer, found %s instead", optarg);
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
    printf("Terminate chance: %i%%\n", terminate_chance);
    if (verbose)
        printf("Verbose logfile entries are on.\n");
    sleep(2);
    
    /* I just felt like putting this in a separate method so it clears up main a little bit. */
    setupSharedMemory();
    
    //MARK: Process creation while loop
    int max_processes = 18, process_count = 0, process_number = 1;
    char procID[10], process_creation_time[15], logical_run_time[3], term_chance[4], verboseEntries[2];
    sprintf(term_chance, "%i", terminate_chance);
    if (verbose) {
        sprintf(verboseEntries, "1");
    } else {
        sprintf(verboseEntries, "0");
    }
    pid_t wpid, pid;
    int status;
    srand((unsigned)time(NULL));
    alarm(90);
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
            /* 1ms = 1,000,000ns */
            /* Processes should create between 1ms and 500ms */
            next_creation_time = (*seconds * BILLION + *nano_seconds + (1000000 + rand() % 499000001));
            
            process_count++;
            sprintf(procID, "%i", process_number);
            sprintf(process_creation_time, "%.09f", (double)current_time / BILLION);
            sprintf(logical_run_time, "%i", runtime);
            process_number++;
            pid = fork();
            if (pid == 0) {
                execl("./user", procID, process_creation_time, logical_run_time, term_chance, verboseEntries, logfile, (char *)NULL);
                printf("Process didn't exec properly.\n");
                exit(EXIT_FAILURE);
            }
        }
        
        int processID = waitpid(0, NULL, WNOHANG);
        if (processID > 0) {
            jobs_completed++;
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

//MARK: Print Help Menu
void printHelpMenu() {
    printf("'-h' prints this help menu.\n");
    printf("'-v' turns verbose logfile entries on.\n");
    printf("\"-l fileName\" will assign the argument 'fileName' to the variable 'logfile'.\n");
    printf("\"'-t' time\" will assign the argument in time to the variable t.\n\tThis will be the terminate chance. Default is set to 50%%.\n");
    printf("\"'-r' runtime\" will change the logical runtime to the number given.\n");
    printf("\n");
}

//MARK: Setup Shared Memory
void setupSharedMemory() {
    /* Create shared memory for time */
    if((time_memory = shmget(MEMORY_KEY, memory_size, IPC_CREAT | 0777)) == -1) {
        printf("OSS failed to create shared memory. Exiting program...\n");
        perror("OSS shmget time memory:");
        exit(1);
    }
    
    /* Attach to shared memory for time */
    if((seconds = shmat(time_memory, NULL, 0)) == (long long unsigned *)-1) {
        printf("OSS failed to attach to shared memory. Exiting program...\n");
        perror("OSS shmat time memory:");
        exit(1);
    }
    nano_seconds = seconds + 1;
    claim = seconds + 2;
    *seconds = 0;
    *nano_seconds = 0;
    
    /* Create shared memory for the resource control blocks. */
    if ((resource_memory = shmget(RESOURCE_KEY, sizeof(rcb_t) * 20,  IPC_CREAT | 0777)) == -1) {
        printf("OSS failed to create shared memory for the resource control blocks. Exiting program...\n");
        perror("OSS shmget rcb:");
        exit(1);
    }
    
    if ((RCB_array = shmat(resource_memory, NULL, 0)) == NULL) {
        printf("OSS failed to attach to resource memory. Exiting program...\n");
        perror("OSS shmat resource memory:");
        exit(1);
    }
    
    /* Create shared memory for the statistics. */
    if ((stats_memory = shmget(STATS_KEY, sizeof(stats_t), IPC_CREAT | 0777)) == -1) {
        printf("OSS failed to create statistics memory. Exiting program...\n");
        perror("OSS shmget statistics memory:");
        exit(1);
    }
    
    if ((myStats = shmat(stats_memory, NULL, 0)) == NULL) {
        printf("OSS failed to attach to statistics memory. Exiting program...\n");
        perror("OSS shmat statistics memory:");
        exit(1);
    }
    
    /* Create shared memory for the resource vector */
    /*
    if ((vector_memory = shmget(VECTOR_KEY, sizeof(int) * 20, IPC_CREAT | 0777)) == -1) {
        printf("OSS failed to create shared memory for the resource vector. Exiting program...\n");
        perror("OSS shmget vector:");
        exit(1);
    }
    
    if ((resourceVector = shmat(vector_memory, NULL, 0)) == NULL) {
        printf("OSS failed to attach to the resource vector. Exiting program...\n");
        perror("OSS shmat vector memory:");
        exit(1);
    }
    */
    /* Set up memory for the resource queues */
    if ((queue_memory = shmget(QUEUE_KEY, sizeof(int) * 360, IPC_CREAT | 0777)) == -1) {
        printf("OSS failed to create shared memory for the resource queue. Exiting program...\n");
        perror("OSS shmget queue memory:");
        exit(1);
    }
    
    if ((resourceQueue = shmat(queue_memory, NULL, 0)) == NULL) {
        printf("OSS failed to attach to the resource queue. Exiting program...\n");
        perror("OSS shmat queue memory:");
        exit(1);
    }
    
    /* Set up initial resources for the resource control blocks */
    setupResourceBlocks();
    
    /* Create a named semaphore */
    resource_sem = sem_open("/mySem", O_CREAT, 0777, 1);
    /* Check if the semaphore created correctly */
    if(resource_sem == SEM_FAILED) {
        printf("Semaphore failed to create. Exiting program...\n");
        perror("parent sem_open");
        exit(1);
    }
}

/* Detach from and release all shared memory */
//MARK: Detach Memory
void detachMemory() {
    double current_time = *seconds + (double)*nano_seconds / BILLION;
//    printf("Time on deck: %.09f seconds.\n", current_time);
    /* Statistics to print: */
    printf("\nJobs completed: %i\n", jobs_completed);
    if (jobs_completed > 0)
        printf("Throughput: %f jobs/minute.\n", (current_time / jobs_completed) * 60);
    else
        printf("No jobs completed.\n");
    printf("Turnaround time: %f second(s).\n", current_time);
    printf("Waiting time: %f\n", myStats->turnaround_time / jobs_completed);
    printf("CPU utilization: %.02f%%\n", 100 - (myStats->cpu_utilization / current_time) * 100);
    
    /* Detach and release the memory for the statistics */
    shmdt(myStats);
    shmctl(stats_memory, IPC_RMID, NULL);
    
    /* Unlink and close the semaphore. Unlink ONLY happens here, not in child processes. */
    sem_unlink("/mySem");
    sem_close(resource_sem);
    
    /* Detach and release the memory for the resource blocks */
    shmdt(RCB_array);
    shmctl(resource_memory, IPC_RMID, NULL);
    
    /* Detach and release the memory for the resource vector */
//    shmdt(resourceVector);
//    shmctl(vector_memory, IPC_RMID, NULL);
    
    /* Detach and release the memory for the resource queue */
    shmdt(resourceQueue);
    shmctl(queue_memory, IPC_RMID, NULL);
    
    /* Detach and release the memory for time */
    shmdt(nano_seconds);
    shmdt(seconds);
    shmdt(claim);
    shmctl(time_memory, IPC_RMID, NULL);
    
    exit(0);
}

//MARK: Signal Handler
void signalHandler(int signal_sent) {
    switch (signal_sent) {
        case 2:
            printf("KNOCK ON MY DOOR, KNOCK NEXT TIME! Did you see anything?!\nNo, sir! I didn't see you playing with your processes again!\n");
            
            fprintf(stderr, "OSS process %i got interrupted while playing with its processes.\n", getpid());
            break;
        case 11:
            printf("Good job, you created a segmentation fault. Are you proud of yourself? ARE YOU?!\n");
            fprintf(stderr, "OSS process %i suffered a segmentation fault. It's dead Jim....\n", getpid());
            break;
        case 14:
            fprintf(stderr, "Time has expired!\n");
            break;
        case 15:
            fprintf(stderr, "SIGTERM sent\n");
            break;
            
        default:
            printf("Good job, you sent the impossible signal.\n");
            break;
    }
    sleep(2);
    killpg(getpgrp(), signal_sent);
    pid_t wpid; int status;
    while ((wpid = wait(&status)) > 0);
    
    detachMemory();
}

/* Run the deadlock detection algorithm and stop processes if necessary. */
//MARK: Deadlock Detection
void deadlockDetection() {
    /* Implement matrix subtraction to determine whether deadlock has occurred. */
    int available[20] = {0};
    /* allocation[] */
    /* claim - allocation = available */
    /* Need to figure out a way to get the claim from each process. */
    sem_wait(resource_sem);
    bool deadlockedResources[20] = {false}, deadlock_occurred;
    /* Run deadlock detection */
    printf("Running deadlock detection...\n");
    int i;
    //TODO: do-while loop for deadlock detection
    /* Figure out what the total claim is. */
    
    /* Determine what the available resources are. */
    do {
        deadlock_occurred = false;
        for (i = 0; i < 20; i++) {
            available[i] = RCB_array[i].maxResourceCount - claim[i];
            
            if (available[i] < 0) {
                deadlockedResources[i] = true;
                deadlock_occurred = true;
            }
            printQueue(i);
            /* Whenever this number is negative it means that we have processes waiting for resources. */
            printf("available[%i]: %i\n", i, available[i]);
        }   /* For loop to determine if a deadlock has occurred. */
        sleep(2);
        /* Write to files if necessary. */
        if (deadlock_occurred) {
            FILE *file;
            file=fopen(logfile, "a");
            if (file == NULL) {
                fprintf(stderr, "Failed to open file, exiting program.\n");
                errno = ENOENT;
                signalHandler(SIGSEGV);
                exit(1);
            }
            /* Start killing processes */
            for (i = 0; i < 20; i++) {
                /* The process that is  */
                if (deadlockedResources[i] && resourceQueue[i*20] > 0) {
                    fprintf(file, "Deadlock has occurred at time %.09f, taking corrective action...\n", *seconds + (double)*nano_seconds / BILLION);
                    fprintf(file, "Killing process %i from resource queue %i\n", resourceQueue[i*20], i);
                    kill(resourceQueue[i * 20], SIGTERM);
                    deadlockedResources[i] = false;
                    sleep(1);
                }
            }
            fclose(file);
        }
    } while (deadlock_occurred);
    printf("Deadlock detection finished.\n");
    sleep(2);
    sem_post(resource_sem);
}

/* Set up the resources blocks. */
//MARK: Setup Resource Blocks
void setupResourceBlocks() {
    printf("Setting up resource blocks...\n");
    srand((unsigned)time(NULL));
    int i = 0, willBeShareable = 15 + rand() % 11;
    printf("willBeShareable is set to %i\n", willBeShareable);
    for (; i < 20; i++) {
        if (rand() % 100 <= willBeShareable) {
            RCB_array[i].isShareable = true;
            printf("Resource %i is shareable\n", i);
        } else {
            RCB_array[i].isShareable = 0;
        }
        
        /* What I have been told about shareable resources is that a request will always be granted. I'll have to do some additional setup in user so it won't request more than 10 if the resource is shareable. */
        if (RCB_array[i].isShareable) {
            RCB_array[i].maxResourceCount = 180;
        } else {
            RCB_array[i].maxResourceCount = 1 + rand() % 10;
//            RCB_array[i].maxResourceCount = 1;
        }
        RCB_array[i].currentResourceCount = RCB_array[i].maxResourceCount;
    }
    printf("And now to continue with your regularly scheduled program\n");
    sleep(2);
}

//MARK: Print Queue
void printQueue(int index) {
    printf("Queue %i\n", index);
    int i;
    for (i = 0; i < 18; i++) {
        if (resourceQueue[index * 20 + i] == 0)
            break;
        else
            printf("%i -> ", resourceQueue[index * 20 + i]);
    }
    if (i > 0)
        printf("\n");
}
