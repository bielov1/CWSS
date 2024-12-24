#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "driver.h"

Buffer active_buffer = {0};
Buffer schedule_queue[REQUESTS_NUM] = {0};
Buffer schedule_queue2[REQUESTS_NUM] = {0};

SleepQueue sleep_queue[REQUESTS_NUM] = {0};

Disk_Controller* dc = NULL;
//----------------------------------------------------------------------------
void initialize_dc()
{
    dc = (Disk_Controller*) malloc(sizeof(Disk_Controller));
    if (dc == NULL)
    {
        fprintf(stderr, "Memory allocation for cache failed!\n");
        exit(1);
    }

    dc->head_pos = 0;
    dc->head_direction = 1;
}
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
void print_device_strategy(const char* strategy, SchedulerType sched_t)
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

    if (sched_t == SCHEDULER_FLOOK)
    {
	printf("\tSchedule queue_2 [");
	for (int i = 0; i < REQUESTS_NUM; ++i)
	{
	    if (schedule_queue2[i].used)
	    {
		printf("(%ld:%ld), ", schedule_queue2[i].process.track, schedule_queue2[i].process.sector);
	    }
	}
	printf("]\n");
    }
}
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
void delete_node(IORequestNode **list_p, IORequestNode *curr_request)
{
    if (*list_p == NULL || curr_request == NULL) return;

    if (*list_p == curr_request) *list_p = curr_request->next;

    if (curr_request->next != NULL) curr_request->next->prev = curr_request->prev;

    if (curr_request->prev != NULL) curr_request->prev->next = curr_request->next;

    free(curr_request);
}
//----------------------------------------------------------------------------
void schedule_process_as_buffer(Buffer *queue, Process *process)
{
    printf("[DRIVER] Buffer (%ld:%ld) scheduled\n", process->track, process->sector);
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
        if (!queue[i].used)
        {
            queue[i].counter = 1;
            queue[i].process = *process;
            queue[i].used = true;
            queue[i].active = false;
	    return;
        }
    }
}
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
bool active_buffer_exists()
{
    return active_buffer.active;
}
//----------------------------------------------------------------------------
bool process_is_active_buffer(Process *process)
{
    return ((active_buffer.process.sector == process->sector) && (process->waits_for_next_interrupt > 0));
}
//----------------------------------------------------------------------------
SleepQueue* get_sleep_q_process()
{
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	if (sleep_queue[i].state == CHECK)
	{
	    return &sleep_queue[i];
	}
    }

    return NULL;
}
//----------------------------------------------------------------------------
void complete_sleep_queue_process(SleepQueue* elem)
{
    elem->state = EXITED;
}
//----------------------------------------------------------------------------
void complete_process(Process complete_process, long int time_spent)
{
    if (strcmp(complete_process.action, "READ") == 0)
    {
	for (int i = 0; i < REQUESTS_NUM; ++i)
	{
	    if (!sleep_queue[i].used)
	    {
		sleep_queue[i].time = time_spent + AFTER_READING_TIME;
		sleep_queue[i].process = complete_process;
		sleep_queue[i].state = CHECK;
		sleep_queue[i].used = true;
		break;
	    }
	}
    }
    if (active_buffer.active)
    {
	cache_put(&active_buffer);
	free_active_buffer();
    }
    cache_print();
}
//----------------------------------------------------------------------------
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
	    if (dc->head_pos < track)
	    {
		dc->head_pos++;
		time += TRACK_SEEK_TIME;
	    }
	    else
	    {
		dc->head_pos--;
		time += TRACK_SEEK_TIME;
	    }
	}
    }

    SleepQueue* compl_process = get_sleep_q_process();
    if (compl_process != NULL)
    {
	long int move_wait = time;
	completed_process_waits_for_check(compl_process, move_wait, time_worked);
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
//----------------------------------------------------------------------------
void free_active_buffer()
{
    active_buffer.counter = 1;
    active_buffer.process.waits_for_next_interrupt = -1;
    active_buffer.used = false;
    active_buffer.active = false;
}
//----------------------------------------------------------------------------
void set_process_as_active_buffer(Process *process)
{    
    active_buffer.counter = 1;
    active_buffer.process = *process;
    active_buffer.used = true;
    active_buffer.active = true;
}
//----------------------------------------------------------------------------
Buffer* get_schedule_queue()
{
    return schedule_queue;
}
//----------------------------------------------------------------------------
Buffer* get_schedule_queue2()
{
    return schedule_queue2;
}
//----------------------------------------------------------------------------
Buffer* get_active_buffer()
{
    return &active_buffer;
}
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
bool schedule_queue_is_sorted = false;

int compare_buffers(const void *a, const void *b) {
    const Buffer *buffer_a = (const Buffer *)a;
    const Buffer *buffer_b = (const Buffer *)b;

    if (buffer_a->process.sector < buffer_b->process.sector)
    return -1;
    else if (buffer_a->process.sector > buffer_b->process.sector)
    return 1;
    else
    return 0;
}


bool queue_is_empty(Buffer *queue)
{
    for (int i = 0; i < REQUESTS_NUM; i++)
    {
        if (queue[i].used) return false;
    }
    return true;
}

size_t last_track = -1;
int posx = 0;
bool reversed = false;
Process* look_schedule()
{
    qsort(schedule_queue, REQUESTS_NUM, sizeof(Buffer), compare_buffers);
    
    while (!queue_is_empty(schedule_queue))
    {
	if (!reversed)
	{
	    for (int i = posx; i < REQUESTS_NUM; ++i)
	    {
		if (schedule_queue[i].used)
		{
		    if (last_track != schedule_queue[i].process.track)
		    {
			if (find_buffer_in_cache(&schedule_queue[i]))
			{
			    schedule_queue[i].process.duplicate = true;
			}
			
			schedule_queue[i].used = false;
			last_track = schedule_queue[i].process.track;
			posx = i+1;
			return &schedule_queue[i].process;	    
		    }
		    else
		    {
			printf("[DRIVER] Too many I/O requests for one track, skip buffer (%ld:%ld)\n", schedule_queue[i].process.track, schedule_queue[i].process.sector);
		    }
		}
	    }
	    posx = REQUESTS_NUM - 1;
	    reversed = true;
	}
	else
	{
	    for (int i = posx; i >= 0; i--)
	    {
		if (schedule_queue[i].used)
		{
		    if (last_track != schedule_queue[i].process.track)
		    {
			if (find_buffer_in_cache(&schedule_queue[i]))
			{
			    schedule_queue[i].process.duplicate = true;
			}
			
			schedule_queue[i].used = false;
			last_track = schedule_queue[i].process.track;
			posx = i-1;
			return &schedule_queue[i].process;
		    }
		    else
		    {
			printf("[DRIVER] Too many I/O requests for one track, skip buffer (%ld:%ld)\n", schedule_queue[i].process.track, schedule_queue[i].process.sector);
		    }
		}
	    }
	    posx = 0;
	    reversed = false;
	}

	// in case when last_track has the same track number as last unused buffer in schedule.
	// This may cause infinite loop.
	last_track = -1;
    }

    return NULL;
}
//----------------------------------------------------------------------------
Buffer *active_queue = schedule_queue;
Buffer *inactive_queue = schedule_queue2;

bool not_in_queue(Buffer *queue, size_t sector)
{
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	if (sector == queue[i].process.sector)
	{
	    return false;
	}
    }
    return true;
}

bool can_add_process_in_second_queue(IORequestNode* requests)
{
    IORequestNode* next_req = requests->next;
    bool first_interrupt_set = false;
    while (next_req != NULL)
    {
	if (not_in_queue(active_queue, next_req->process->sector) && not_in_queue(inactive_queue, next_req->process->sector))
	{
	    if (strcmp(active_buffer.process.action, next_req->process->action) != 0)
	    {
		if (!first_interrupt_set)
		{
		    first_interrupt_set = true;
		    next_req->process->state = WAITING_FOR_INTERRUPT;
		}
	    }
	    else
	    {
		return true;
	    }
	}
	next_req = next_req->next;
    }
    return false;
}

bool fill_inactive_queue = true;
Process* flook_schedule(IORequestNode *req)
{
    
    if (queue_is_empty(active_queue))
    {
	//clear_queue(active_queue);
	fill_inactive_queue = !fill_inactive_queue;
        Buffer *tmp = active_queue;
        active_queue = inactive_queue;
        inactive_queue = tmp;


        if (queue_is_empty(active_queue)) {
            return NULL;
        }
    }

    qsort(active_queue, REQUESTS_NUM, sizeof(Buffer), compare_buffers);

    int selected_index = -1;
    int curr_dc_head_pos = dc->head_pos;
    while (selected_index < 0)
    {
	for (int i = 0; i < REQUESTS_NUM; i++)
	{
            if (active_queue[i].used && (int)active_queue[i].process.track >= curr_dc_head_pos)
            {
		selected_index = i;
		break;
            }
	}

	curr_dc_head_pos = 0;
    }

    if (can_add_process_in_second_queue(req))
    {
	IORequestNode* next_req = req->next;
	while (next_req != NULL)
	{
	    if (fill_inactive_queue)
	    {
		if (not_in_queue(active_queue, next_req->process->sector) && not_in_queue(inactive_queue, next_req->process->sector))
		{
		    if (strcmp(active_buffer.process.action, next_req->process->action) == 0)
		    {
			read_process(next_req->process);
			next_req->process->state = BLOCKED;
			printf("[DRIVER] Buffer (%ld:%ld) was added to inactive queue\n", next_req->process->track, next_req->process->sector);
			schedule_process_as_buffer(inactive_queue, next_req->process);
			break;
		    }
		}
	    }

	    next_req = next_req->next;
	}
    }

    if (find_buffer_in_cache(&active_queue[selected_index]))
    {
	active_queue[selected_index].process.duplicate = true;
    }
    active_queue[selected_index].used = false;
    return &active_queue[selected_index].process;
}
//----------------------------------------------------------------------------
