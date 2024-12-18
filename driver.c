#include <stdio.h>
#include <stdlib.h>

#include "driver.h"

Buffer active_buffer = {0};
Buffer schedule_queue[REQUESTS_NUM] = {0};
Buffer schedule_queue2[REQUESTS_NUM] = {0};
Disk_Controler* dc = NULL;

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

void print_device_strategy(const char* strategy)
{
    printf("[DRIVER] Device strategy ");
    printf("%s", strategy);
    printf("\n");
    if (active_buffer_exists())
    {
	printf("\tActive_buffer (%ld:%ld)\n", active_buffer.process.track, active_buffer.process.sector);
    }

    printf("\tSchedule queue_1 [");
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	if (schedule_queue[i].used)
	{
	    printf("(%ld:%ld), ", schedule_queue[i].process.track, schedule_queue[i].process.sector);
	}
    }
    printf("]\n");
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

void schedule_process_as_buffer(Process *process)
{
    printf("[DRIVER] Buffer (%ld:%ld) scheduled\n", process->track, process->sector);
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
        if (!schedule_queue[i].used)
        {
            schedule_queue[i].counter = 1;
            schedule_queue[i].process = *process;
            schedule_queue[i].used = true;
            schedule_queue[i].active = false;
	    return;
        }
    }
}

bool schedule_queue_is_empty()
{
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	if (schedule_queue[i].used)
	{
	    return false;
	}
    }

    return true;
}

bool active_buffer_exists()
{
    return active_buffer.active;
}

bool process_is_active_buffer(Process *process)
{
    return ((active_buffer.process.sector == process->sector) && (process->waits_for_next_interrupt > 0));
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

void move_arm_to_track(Process *p, long int *time_worked)
{
    long int time = 0;
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
	printf("\tmove time %ld\n", time);
    }
    
    *time_worked += time;
    
    generate_interrupt(p, *time_worked);
}

void free_active_buffer()
{
    active_buffer.counter = 1;
    active_buffer.process = (Process){0};
    active_buffer.process.waits_for_next_interrupt = -1;
    active_buffer.used = false;
    active_buffer.active = false;
}

void set_process_as_active_buffer(Process *process)
{    
    active_buffer.counter = 1;
    active_buffer.process = *process;
    active_buffer.used = true;
    active_buffer.active = true;
}

Buffer sleep_q[REQUESTS_NUM] = {0};

// fifo algo fills up sleep_q with processes, that was retrieved from schedule_queue untill its empty.
Process* fifo_schedule()
{
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	if (schedule_queue[i].used)
	{
	    if (find_buffer_in_cache(&schedule_queue[i]))
	    {
		schedule_queue[i].process.duplicate = true;
	    }
	    schedule_queue[i].used = false;
	    return &schedule_queue[i].process;
	}
    }
    
    return NULL;
}
