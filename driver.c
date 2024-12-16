#include <stdio.h>
#include <stdlib.h>

#include "driver.h"

bool active_buffer_exists = false;
Buffer active_buffer = {0};
Buffer schedule_queue[REQUESTS_NUM];
Buffer schedule_queue2[REQUESTS_NUM];
Disk_Controler* dc = NULL;

// to know what process serve next after fifo algo
Process *next_process_to_serve = NULL;

void initialize_dc()
{
    dc = (Disk_Controler*) malloc(sizeof(Disk_Controler));
    if (dc == NULL)
    {
        fprintf(stderr, "Memory allocation for cache failed!\n");
        exit(1);
    }

    dc->head_pos = 0;
    dc->head_direction = 1;
}

void intialize_schedule_queue()
{
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	schedule_queue[i].counter = 0;
	schedule_queue[i].sector = -1;
	schedule_queue[i].track = -1;
	schedule_queue[i].used = false;
	schedule_queue[i].active = false;
    }
}

int add_request(IORequestNode **list_p, Process *p)
{
    IORequestNode *new_request;

    new_request = (IORequestNode*) malloc(sizeof(IORequestNode));
    new_request->process = p;
    new_request->next = *list_p;
    new_request->prev = NULL;

    if (*list_p != NULL) (*list_p)->prev = new_request;

    *list_p = new_request;

    return 0;
}


void print_request_queue(IORequestNode *list)
{
    IORequestNode *request;

    if (list == NULL) printf("[empty]\n");
    else
    {
	printf("[");
	request = list;
	while (request != NULL)
	{
	    printf("%ld ", request->process->sector);
	    request = request->next;
	}

	printf("]\n");
    }
}

void print_device_strategy(SchedulerType sched_t)
{
    printf("[DRIVER] Device strategy ");
    if (sched_t == SCHEDULER_FLOOK)
    {
	printf("FLOOK\n");
	if (active_buffer_exists)
	{
	    printf("\tActive_buffer (%ld:%ld)\n", active_buffer.track, active_buffer.sector);
	}

	printf("\tSchedule queue_1 [");
	for (int i = 0; i < REQUESTS_NUM; ++i)
	{
	    if (schedule_queue[i].used)
	    {
		printf("(%ld:%ld), ", schedule_queue[i].track, schedule_queue[i].sector);
	    }
	}
	printf("]\n");
    }
}

void reverse_queue(IORequestNode **list_p)
{
    IORequestNode *temp = NULL;
    IORequestNode *curr = *list_p;

    while (curr != NULL)
    {
	temp = curr->prev;
	curr->prev = curr->next;
	curr->next = temp;
	curr = curr->prev;
    }

    if (temp != NULL) *list_p = temp->prev;
}

void delete_node(IORequestNode **list_p, IORequestNode *curr_request)
{
    if (*list_p == NULL || curr_request == NULL) return;

    if (*list_p == curr_request) *list_p = curr_request->next;

    if (curr_request->next != NULL) curr_request->next->prev = curr_request->prev;

    if (curr_request->prev != NULL) curr_request->prev->next = curr_request->next;

    free(curr_request);
}

void schedule_buffer(Buffer *buffer)
{
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
        if (!schedule_queue[i].used)
        {
            schedule_queue[i].counter = buffer->counter;
            schedule_queue[i].sector = buffer->sector;
            schedule_queue[i].track = buffer->track;
            schedule_queue[i].used = true;
            schedule_queue[i].active = buffer->active;
	    return;
        }
    }
}

bool process_is_active_buffer(Process *p)
{
    return (active_buffer.sector == p->sector);
}

void complete_process()
{
    if (active_buffer.active)
    {
	cache_put(&active_buffer);
	free_active_buffer();
    }
    cache_print();
}

void move_arm_to_track(Process *p, int *time_worked)
{
    int time = 0;
    int track = p->sector/SECTORS_PER_TRACK;
    printf("[DRIVER] Best move decision for tracks %d => %d\n", dc->head_pos, track);
    while (dc->head_pos != track)
    {
	
	if (dc->head_pos >= TRACKS)
	{
	    dc->head_pos = 0;
	}
	else
	{
	    dc->head_pos++;
	    time += TRACK_SEEK_TIME;
	}
    }
    
    if (time == 0)
    {
	printf("\tnot to move, that is 0 us\n");
    }
    else
    {
	printf("\tmove time %d\n", time);
    }
    
    *time_worked += time;
    
    generate_interrupt(p, *time_worked);
}

void free_active_buffer()
{
    active_buffer_exists = false;

    active_buffer.counter = 1;
    active_buffer.sector = -1;
    active_buffer.track = -1;
    active_buffer.used = false;
    active_buffer.active = false;
}

void set_active_buffer(Buffer *buffer)
{
    active_buffer_exists = true;
    buffer->active = true;
    
    active_buffer.counter = buffer->counter;
    active_buffer.sector = buffer->sector;
    active_buffer.track = buffer->track;
    active_buffer.used = buffer->used;
    active_buffer.active = buffer->active;
}

int syscall_read(Process *p, int *time_spent)
{
    Buffer *out_buffer;

    if (!find_buffer_in_cache(p->sector, &out_buffer))
    {
        Buffer *buf = get_free_buffer_cache();
        if (buf == NULL) {
            fprintf(stderr, "No suitable buffer found\n");
            exit(1);
        }
        buf->counter = 1;
        buf->sector = p->sector;
        buf->track = p->sector / SECTORS_PER_TRACK;
        buf->used = false;
        buf->active = false;
        
        out_buffer = buf;
    }
    else
    {
	p->state = WAKEUP;
	return 1;
    }

    if (!active_buffer_exists)
    {
	set_active_buffer(out_buffer);
	move_arm_to_track(p, time_spent);
    }
    else
    {
	printf("[DRIVER] Scheduled process %ld\n", p->sector);
	p->state = SCHEDULED;
	schedule_buffer(out_buffer);
    }
    return 0;
}

Process sleep_q[REQUESTS_NUM] = {0};

// fifo algo fills up sleep_q with processes, that was retrieved from schedule_queue untill its empty.
void fifo_schedule()
{
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	if (schedule_queue[i].used)
	{
	    next_process_to_serve->sector = schedule_queue[i].sector;
	    schedule_queue[i].used = false;
	    return;
	}
    }
}

bool is_next_process_to_serve(Process *p)
{    
    if (next_process_to_serve == NULL)
    {
	next_process_to_serve = (Process*) malloc(sizeof(Process));
    }

    return (p->sector == next_process_to_serve->sector);
}
