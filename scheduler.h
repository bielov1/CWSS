#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "hdd.h" // config process
#include "cache_lfu.h"
#include "driver.h"


int fifo_schedule(IORequestNode **rq);
//int look_schedule();
bool flook_schedule(Buffer* schedule_queue, Buffer *next_buffer);

//void print_current_time()
void read_process(Process *curr_process);
void tick(IORequestNode *curr_request, int *time_spent);

#endif
