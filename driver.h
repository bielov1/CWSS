#ifndef DRIVER_H_
#define DRIVER_H_

#include "process.h"
#include "cache_lfu.h"
#include "scheduler.h"

typedef struct {
    int head_pos;
    int head_direction;
} Disk_Controller;

typedef struct ScheduleQueueStruct {
    Buffer* buffer;
    struct ScheduleQueueStruct *prev;
    struct ScheduleQueueStruct *next;
} ScheduleQueueNode;

void initialize_dc();
int add_request(IORequestNode **list_p, Process* p);
void print_request_queue(IORequestNode *list);
void print_device_strategy(const char* strategy, SchedulerType sched_t);
void reverse_queue(IORequestNode **list_p);
void delete_node(IORequestNode **list_p, IORequestNode *curr_request);
void schedule_process_as_buffer(Buffer *queue, Process *process);
bool schedule_queue_is_empty();

bool active_buffer_exists();
bool process_is_active_buffer(Process *process);
bool queue_is_empty(Buffer *queue);

void complete_process();
void move_arm_to_track(Process *p, long int *time_worked);
void free_active_buffer();
void set_process_as_active_buffer(Process *process);

Buffer* get_schedule_queue();
Buffer* get_active_buffer();
Buffer* get_schedule_queue2();

Process* fifo_schedule();
Process* look_schedule();
Process* flook_schedule(IORequestNode *req);
#endif
