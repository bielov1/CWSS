#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "scheduler.h"
#include "driver.h"
#include "cache_lfu.h"

int next_interrupt = -1;

bool handle_interrupt(int *time)
{
    if (*time == next_interrupt) {
        printf("[INTERRUPT] Handling interrupt at %d us\n", *time);
        next_interrupt = -1;
        return true;
    }
    return false;
}

void generate_interrupt(int *time_worked)
{
    next_interrupt = *time_worked + 4016;
    printf("[INTERRUPT] Next interrupt at %d\n", next_interrupt); 
}

void sort_io_request_node(IORequestNode **head) {
    if (!head || !*head) return;
    
    IORequestNode *sorted = NULL;
    IORequestNode *current = *head;
    
    while (current) {
        IORequestNode *next = current->next;
        
        if (!sorted || sorted->process->sector >= current->process->sector) {
            current->next = sorted;
            current->prev = NULL;
            if (sorted) sorted->prev = current;
            sorted = current;
        } else {
            IORequestNode *temp = sorted;
            while (temp->next && temp->next->process->sector < current->process->sector) {
                temp = temp->next;
            }
            current->next = temp->next;
            current->prev = temp;
            if (temp->next) temp->next->prev = current;
            temp->next = current;
        }
        current = next;
    }
    
    *head = sorted;
}


int fifo_schedule(IORequestNode **rq)
{
    reverse_queue(rq);
    return 0;
}

int flook_schedule1(IORequestNode **rq, Disk_Controler *dc, int *time_worked)
{
    IORequestNode *first_q = NULL;
    IORequestNode *second_q = NULL;
    IORequestNode *active_queue;
    IORequestNode *active_request = NULL;
    IORequestNode *rq_queue;
    IORequestNode *rq_request;

    int served_processes = 0;
    
    print_request_queue(*rq);

    rq_queue = *rq;
    rq_request = rq_queue;
    active_queue = first_q;

    srand(time(NULL));
    while (rq_request != NULL || active_queue != NULL)
    {
	printf("[SCHEDULER] %d us (NEXT ITERATION)\n", *time_worked);
	//*(time_worked)++;
	if (active_queue == NULL)
	{
	    int i;
	    int limit = rand() % 5 + 1;
	    while (served_processes + limit > REQUESTS_NUM) limit--;

	    for (i = 0; i < limit && rq_request != NULL; ++i)
	    {
		add_request(&active_queue, rq_request->process);
		rq_request = rq_request->next;
	    }
	    served_processes += i;
	    sort_io_request_node(&active_queue);
	}
	
	active_request = active_queue;
	//read_process(active_request);
	
	if (handle_interrupt(time_worked)) {
            if (active_request != NULL) {
		active_request->process->state = WAKEUP;
		printf("[SCHEDULER] Process %ld wake up\n", active_request->process->sector);
            }
	}

	if (active_request->process->mode == USER_MODE)
	{
	    change_process_mode(active_request, KERNEL_MODE);
	}
	else
	{
	    Buffer active_request_buffer = request_buffer_cache(active_request);

	    if (!active_request_buffer.used)
	    {
		if (set_active_buffer(&active_request_buffer, active_request->process->is_reading) )
		{
		    int move_time = move_arm_to_track(active_request_buffer.track, dc, time_worked);
		    *time_worked += move_time;
		    printf("[SCHEDULER] Block process %ld\n", active_request->process->sector);
		    active_request->process->state = BLOCKED;
		}
		
		if (active_request->process->state == WAKEUP)
		{
		    cache_put(&active_request_buffer);
		    delete_node(&active_queue, active_request);
		}
	    }
	    cache_print();
	    print_device_strategy();
	}
	
	if (active_queue == NULL)
	{
	    if (active_queue == first_q) active_queue = second_q;
            else active_queue = first_q;
	}
	
	
    }
    printf("[SCHEDULER] %d us (NEXT ITERATION)\n", *time_worked);
    printf("[SCHEDULER] Scheduler has nothing to do, exit\n");
    return 0;
}

int flook_schedule(IORequestNode **request_queue, Disk_Controler *dc, int *time_worked)
{
    IORequestNode *first_q = NULL;
    IORequestNode *second_q = NULL;
    IORequestNode *active_queue = first_q;
    IORequestNode *active_request = NULL;

    bool last_request_action = (*request_queue)->process->is_reading;
    
    while (*request_queue != NULL)
    {

	while (last_request_action && (*request_queue)->process->is_reading)
	{
	    add_request(&active_queue, (*request_queue)->process);
	    *request_queue = (*request_queue)->next;
	}
	
	sort_io_request_node(&active_queue);
	active_request = *active_queue;
	
	
	while (active_queue != NULL)
	{
	    Buffer active_request_buffer = request_buffer_cache(active_request);
	    
	    if (!set_active_buffer(&active_request_buffer))
	    {
		add_buffer_to_sleep_schedule(active_request_buffer, last_request_action);
	    }
	    else
	    {
		move_arm_to_track(active_request_buffer.track, last_request_action, time_worked);
	    }
	}
	
	last_request_action = (*request_queue)->process->is_reading;
	
	if (active_queue == first_q) active_queue = second_q;
        else active_queue = first_q;

    }
    
}


/*
void look_scheduler(Queue *q, Disk_Controler *dc)
{
    // start point
    bool reverse = false;
    int requests_count = 0;
    int end = q->rear - 1;
    quick_sort_queue(q, q->front + 1, end);
    while (!is_empty(q))
    {
	bool found_request = false;
	int queue_start = q->front + 1;
	int queue_end = q->rear - 1;
	if (reverse)
	{
	    for (int i = queue_end; i >= queue_start; i--)
	    {
		if (dc->current_track == q->items[i].track)
		{
		    send_process_to_hdd(q->items[i]);
		    dequeue_tail(q);
		    requests_count++;
		    found_request = true;

		    // limit check for current track to process requests that is in the same track.
		    if (requests_count >= MAX_REQUESTS_PER_TRACK)
		    {
			break;
		    }
		}
	    }
	}
	else
	{
	    for (int i = queue_start; i <= queue_end; ++i)
	    {
		if (dc->current_track == q->items[i].track)
		{
		    send_process_to_hdd(q->items[i]);
		    dequeue_head(q);
		    requests_count++;
		    found_request = true;

		    // limit check for current track to process requests that is in the same track.
		    if (requests_count >= MAX_REQUESTS_PER_TRACK)
		    {
			break;
		    }
		}
	    }
	}
	
	if (!found_request || requests_count >= MAX_REQUESTS_PER_TRACK)
        {
            requests_count = 0;
            dc->current_track = reverse ? q->items[q->front + 1].track : q->items[q->rear - 1].track;
	    reverse = !reverse;
        }
    }
    
}

*/
