#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "scheduler.h"
#include "driver.h"
#include "cache_lfu.h"

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

int flook_schedule(IORequestNode **rq, Disk_Controler *dc, int *time_worked)
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
	read_process(active_request);
	
	if (active_request->process->mode == USER_MODE)
	{
	    change_process_mode(active_request, KERNEL_MODE);
	}
	else
	{
	    Buffer *active_request_buffer = request_buffer_cache(active_request);
	    int move_time = move_arm_to_track(active_request_buffer->track, dc);
	    *time_worked += move_time;
	    printf("removed %ld node, took %d us to move\n", active_request->process->sector, move_time);
	    delete_node(&active_queue, active_request);
	    //TODO: needs other logic in case when buffer was found in cache
	    // maybe, don't call cache_put()
	    cache_put(active_request_buffer);
	}
	
	if (active_queue == NULL)
	{
	    if (active_queue == first_q) active_queue = second_q;
            else active_queue = first_q;
	}
	
	
    }
    
    
    return 0;
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
