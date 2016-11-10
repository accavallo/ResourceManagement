//
//  main.c
//  Proj5
//
//  Created by Tony on 11/2/16.
//  Copyright © 2016 Anthony Cavallo. All rights reserved.
//

#include "Proj5.h"

int main(int argc, const char * argv[]) {
    
    
    if((memory = shmget(KEY, sizeof(int), IPC_CREAT | 0777)) == -1) {
        printf("OSS failed to create shared memory. Exiting program...\n");
        exit(1);
    }
    
    if (fork() == 0) {
        execl("./user", (char *)NULL);
    }
    
    detachMemory();
    return 0;
}

void detachMemory() {
    shmctl(memory, IPC_RMID, NULL);
}
