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
    
    sleep(1);
    printf("Hello, from process %i!\n", getpid());
    
    return 0;
}

void detachMemory() {
    
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
    sleep(1);
    printf("User process %i is being put away because time is up.\n", getpid());
    
    exit(0);
}
