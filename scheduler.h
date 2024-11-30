#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "hdd.h"

int fifo_schedule(IORequestNode **rq);
//int look_schedule();
int flook_schedule(IORequestNode **rq, Disk_Controler *dc, int *time_worked);

#endif
