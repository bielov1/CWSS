#ifndef KERNEL_H_
#define KERNEL_H_

#include "hdd.h" // config process
#include "driver.h"
#include "scheduler.h"

typedef struct {
    size_t sector;
    long int finish_time;
    bool used;
} ProcessFinishTime;

typedef struct {
    long int time;
} TotalTime;

#endif
