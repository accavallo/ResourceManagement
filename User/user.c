//
//  main.c
//  User
//
//  Created by Tony on 11/2/16.
//  Copyright © 2016 Anthony Cavallo. All rights reserved.
//

#include "Proj5.h"
//resources[i][0] will be the number of resources currently claimed. resources[i][1] will be the number of resources desired to be claimed.
int resources[20][2] = {0}; /* Used to keep track of this process' resource claims */
int processID;

int main(int argc, const char * argv[]) {
    signal(SIGINT, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGALRM, signalHandler);
    signal(SIGTERM, signalHandler);
    /***********
     argv[0] is the process ID as given by oss.
     argv[1] is the creation time of the process.
     argv[2] is the logical run time for oss.
     argv[3] is the chance that the child process will terminate.
     ***********/
    
    int terminate_chance = atoi(argv[3]);
    processID = atoi(argv[0]);
    /* Get shared memory for time */
    if((time_memory = shmget(MEMORY_KEY, memory_size, 0777)) == -1) {
        printf("User %i (process %i) failed to get shared memory. Exiting program...\n", processID, getpid());
        perror("User shmget time memory:");
        exit(1);
    }
    
    /* Attach to shared memory */
    if ((seconds = shmat(time_memory, NULL, 0)) == (long long unsigned *)-1) {
        printf("User %i (process %i) failed to attach to shared memory. Exiting program...\n", processID, getpid());
        perror("User shmat time memory:");
        exit(1);
    }
    nano_seconds = seconds + 1;
    claim = seconds + 2;
    
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
    
    if ((vector_memory = shmget(VECTOR_KEY, sizeof(int) * 20, 0777)) == -1) {
        printf("User %i failed to create shared memory for the resource vector. Exiting program...\n", processID);
        perror("User shmget vector:");
        exit(1);
    }
    
    if ((resourceVector = shmat(vector_memory, NULL, 0)) == NULL) {
        printf("User %i failed to attach to the resource vector. Exiting program...\n", processID);
        perror("User shmat vector memory:");
        exit(1);
    }
    
    /* Open the named semaphore since sem_init() is deprecated... */
    resource_sem = sem_open("/mySem", 0);
    if (resource_sem == SEM_FAILED) {
        perror("User sem_open");
        printf("User %i failed to open semaphore. Exiting program...\n", processID);
        exit(1);
    }
    
    if ((queue_memory = shmget(QUEUE_KEY, sizeof(int) * 360, 0777)) == -1) {
        printf("User %i failed to create shared memory for the resource queue. Exiting program...\n", processID);
        perror("User shmget queue memory:");
        exit(1);
    }
    
    if ((resourceQueue = shmat(queue_memory, NULL, 0)) == NULL) {
        printf("User %i failed to attach to the resource queue. Exiting program...\n", processID);
        perror("User shmat queue memory:");
        exit(1);
    }

//    printf("Hello, from process %3i at time %.09f!\n", processID, *seconds + (double)*nano_seconds / BILLION);
    
    const double CREATION_TIME = atof(argv[1]);
    srand((unsigned)time(NULL));
    double current_time = *seconds + (double)*nano_seconds / BILLION;
    /* Set the first terminate check for at least 1 second after it has been around */
    double terminate_time = current_time + 1 + (rand() % 250000000) / (double)BILLION;
    printf("First terminate time check for %i is set to %.09f.\n", processID, terminate_time + CREATION_TIME);
    bool hasPendingClaim = false, continueLoop = true;
    int amount_to_claim = 0, amount_claimed = 0, resource_to_claim = 0, differnt_resources_claimed = 0;
    /* In this loop there needs to be some resource requests. */
    while (continueLoop) {
        if (hasPendingClaim) {
            /* Keep checking the resource for the claim */
            printf("%i checking on pending claim for resource %i...\n", processID, resource_to_claim);
            sem_wait(resource_sem);
            if (amount_to_claim - amount_claimed <= RCB_array[resource_to_claim].currentResourceCount) {
                hasPendingClaim = false;
                RCB_array[resource_to_claim].currentResourceCount -= (amount_to_claim - amount_claimed);
                resources[resource_to_claim][0] += (amount_to_claim - amount_claimed);
                claim[resource_to_claim] += (amount_to_claim - amount_claimed);
            }
            sem_post(resource_sem);
        } else {
            /* Check between 0 and a bound whether this process will request a new resource or release a resource. */
            if (rand() % 2) {
                sem_wait(resource_sem);
//                    perror("user sem_wait");
                /* Request resource */
                resource_to_claim = (rand() % getpid()) % 20;
                if (RCB_array[resource_to_claim].maxResourceCount > 10) {
                    amount_to_claim = 1 + rand() % 10;
                } else {
                    amount_to_claim = 1 + (rand() % getpid()) % RCB_array[resource_to_claim].maxResourceCount;
                }
                printf("%i is requesting %i from resource %i.\n", processID, amount_to_claim, resource_to_claim);
                sleep(1);
                /* Take the resources requested or up to the amount remaining */
                if (RCB_array[resource_to_claim].currentResourceCount < amount_to_claim) {
                    amount_claimed = RCB_array[resource_to_claim].currentResourceCount;
//                    RCB_array[resource_to_claim].currentResourceCount -= amount_claimed;
                    hasPendingClaim = true;
                    
                } else {
                    amount_claimed = amount_to_claim;
//                    RCB_array[resource_to_claim].currentResourceCount -= amount_to_claim;
//                    resources[resource_to_claim] = amount_to_claim;
//                    claim[resource_to_claim] += amount_to_claim;
                }
                /* Claim the resources that are allowed to be claimed. Whether all that it wants or what is left. */
                RCB_array[resource_to_claim].currentResourceCount -= amount_claimed;    //Subtract what was available.
                resources[resource_to_claim][0] = amount_claimed;  //This process keeps track of what it has actually claimed.
                resources[resource_to_claim][1] = amount_to_claim;
                claim[resource_to_claim] += amount_to_claim;    //Keep track of how much this process wants to claim.
                differnt_resources_claimed++;
//                printf("Process %i: claim[%i] = %llu\tamount_to_claim: %i\tamount_claimed: %i\n", processID, resource_to_claim, claim[resource_to_claim], amount_to_claim, amount_claimed);
//                printf("Process %i: resources[%i]: %i\n", processID, resource_to_claim, resources[resource_to_claim]);
                addToQueue(resource_to_claim);
                sleep(1);
                sem_post(resource_sem);
//                    perror("user sem_post");
            } else {
                sem_wait(resource_sem);
//                    perror("user sem_wait");
                /* Release resource */
                if (differnt_resources_claimed > 0) {
                    int i = 0;
                    for (; i < 20; i++) {
                        if (resources[i] > 0) {
                            RCB_array[i].currentResourceCount += resources[i][0];
                            claim[i] -= resources[i][1];
                            resources[i][0] = 0, resources[i][1] = 0;
                            deleteFromQueue(i);
                            break;
                        }
                    }
                    printf("%i releasing resource from %i.\n", processID, i);
                    differnt_resources_claimed--;
                }
                
                sleep(1);
                sem_post(resource_sem);
//                    perror("user sem_post");
            }
        } /* End pending claim else statement */
        /*Check between 1 ms and 250 ms to determine if the process will finish, after it has been around for at least one second */
        if ((terminate_time + CREATION_TIME) <= current_time) {
            /* Check if this process will terminate */
            if ((1 + rand() % 100) <= terminate_chance) {
                continueLoop = false;
                printf("Releasing memory and resources from %i\n", processID);
                /* Send resources back to the plane from whence they came */
            } else {
                /* Request resources and set up a new time to either terminate or ask for resources */
                terminate_time = current_time + (rand() % 250000000) / (double)BILLION;
                printf("%i trudges on... until at least %.09f\n", processID, terminate_time);
            }
            
        } else if (atoi(argv[2]) <= *seconds)
            continueLoop = false;
        
        /* Update the current time */
        current_time = *seconds + (double)*nano_seconds / BILLION;
    } /* End while loop */
    
    printf("Process %i ending at time %.09f. Last check was at %.09f\n", processID, *seconds + (double)*nano_seconds / BILLION, terminate_time);
    
    detachMemory();
    return 0;
}

void detachMemory() {
    /* Need to go through and release all resources at this point as well as delete the process from the queue. */
    int i = 0; //pid_t pid;
    for (; i < 20; i++) {
        RCB_array[i].currentResourceCount += resources[i][0];
        deleteFromQueue(i);
    }
    
    shmdt(RCB_array);
    shmdt(resourceVector);
    shmdt(resourceQueue);
    shmdt(seconds);
    shmdt(nano_seconds);
    shmdt(claim);
    exit(0);
}

void signalHandler(int signal_sent) {
    pid_t pid = getpid();
    switch (signal_sent) {
        case 2:
            fprintf(stderr, "USER process %i got interrupted while being played with.\n", pid);
            break;
        case 11:
            fprintf(stderr, "USER process %i suffered a segmentation fault. It's still dead Jim....\n", pid);
            break;
        case 14:
            fprintf(stderr, "User process %i is being put away because time is up.\n", pid);
            break;
        case 15:
            fprintf(stderr, "User process %i is being terminated to correct deadlock\n", pid);
            break;
            
        default:
            printf("Good job, you sent the impossible signal.\n");
            break;
    }
    detachMemory();
}

/* Add this process to the correct queue */
void addToQueue(int index) {
    printf("Adding to queue %i\n", index);
    int i = 0;
    for (; i < 18; i++) {
        if (resourceQueue[index * 20 + i] == 0) {
            resourceQueue[index * 20 + i] = getpid();
            break;
        }
    }
}

/* Delete this process from the queue */
void deleteFromQueue(int index) {
    pid_t pid = getpid();
    claim[index] -= resources[index][1];
    /* Need to go through and delete from a very specific queue. */
    int i = 0;
    for (; i < 18; i++) {
        if (resourceQueue[index * 20 + i] == pid) {
            printf("Removing %i from queue %i\n", processID, index);
            resourceQueue[index * 20 + i] = 0;
            break;
        }
    }
    
    /* Need to go and adjust everything so there aren't "holes" in the queue */
    for (i = 0; i < 18; i++) {
        if (resourceQueue[index * 20 + i] == 0 && i != 17) {
            resourceQueue[index * 20 + i] = resourceQueue[index * 20 + i+1];
            resourceQueue[index * 20 + i+1] = 0;
        } else if (resourceQueue[index * 20 + i] == 0 && i == 17)
            break;
    }
}
