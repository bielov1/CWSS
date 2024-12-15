#ifndef DRIVER_H_
#define DRIVER_H_

#include "process.h"
#include "cache_lfu.h"
#include "scheduler.h"


typedef enum {
    SCHEDULER_FIFO,
    SCHEDULER_LOOK,
    SCHEDULER_FLOOK
} SchedulerType;

typedef struct {
    int head_pos;
    int head_direction;
} Disk_Controler;

typedef struct ScheduleQueueStruct {
    Buffer* buffer;
    struct ScheduleQueueStruct *prev;
    struct ScheduleQueueStruct *next;
} ScheduleQueueNode;

void initialize_dc();
void intialize_schedule_queue();
int add_request(IORequestNode **list_p, Process* p);
void print_request_queue(IORequestNode *list);
void print_device_strategy(SchedulerType sched_t);
void reverse_queue(IORequestNode **list_p);
void delete_node(IORequestNode **list_p, IORequestNode *curr_request);
void schedule_buffer(Buffer *buffer);
bool proccess_is_active_buffer(Process *p);
void complete_process();
void move_arm_to_track(Process *p, int *time_worked);
void free_active_buffer();
void set_active_buffer(Buffer *buffer);
void syscall_read(Process *p, int *time_spent);
#endif
