#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "hdd.h"
#include "cache_lfu.h"
#include "driver.h"

void initialize_schedule();

void read_process(Process *curr_process);
int tick(IORequestNode** curr_request, long int *time_spent, SchedulerType sched_t);

#endif
