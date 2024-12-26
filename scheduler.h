#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "hdd.h"
#include "cache_lfu.h"
#include "driver.h"


void initialize_schedule();
int check_sleeping_process(SleepQueue* sp, long int wait_time, long int* time_spent);

void read_process(Process *curr_process);
int tick(IORequestNode** curr_request, long int *time_spent, SchedulerType sched_t);

#endif
