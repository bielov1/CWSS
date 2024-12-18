#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "hdd.h" // config process
#include "cache_lfu.h"
#include "driver.h"

void read_process(Process *curr_process);
int tick(IORequestNode *curr_request, int *time_spent, SchedulerType sched_t);

#endif
