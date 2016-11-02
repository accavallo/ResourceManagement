//
//  main.c
//  Proj5
//
//  Created by Tony on 11/2/16.
//  Copyright © 2016 Anthony Cavallo. All rights reserved.
//

#include "Proj5.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    
    if (fork() == 0) {
        execl("./user", (char *)NULL);
    }
    return 0;
}
