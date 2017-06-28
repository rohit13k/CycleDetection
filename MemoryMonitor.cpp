//
// Created by rohit on 6/28/17.
//

#include "MemoryMonitor.h"
#include <iostream>
#include <sys/resource.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
int parseLine(char* line){
    // This assumes that a digiti will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getMem(){
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmPeak:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result/1024;
}
