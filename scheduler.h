#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "hdd.h" // config process
#include "cache_lfu.h"
#include "driver.h"


void fifo_schedule();
//int look_schedule();
//bool flook_schedule();

//void print_current_time()
void read_process(Process *curr_process);
void tick(IORequestNode *curr_request, int *time_spent, SchedulerType sched_t);

#endif
