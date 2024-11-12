#include "hdd.h"

Queue queues[2];

void initialize_queues()
{
    for (int i = 0; i < 2; ++i)
    {
	initialize_queue(&queues[i]);
    }
}

void quick_sort_queue(Queue *q, int l, int r)
{
    int i = l;
    int j = r;

    Process w;
    
    Process x = q->items[(l+r)/2];
    while (i < j)
    {
	while (q->items[i].sector < x.sector) i++;
        while (x.sector < q->items[j].sector) j--;
     	if (i <= j)
	{
	    w = q->items[i];
	    q->items[i] = q->items[j];
	    q->items[j] = w;
	    i++;
	    j--;
	     
	}
    }
    if (l < j) quick_sort_queue(q, l, j);
    if (i < r) quick_sort_queue(q, i, r);
}

void process_requests(Queue *current_queue, Disk_Controler *dc)
{
    quick_sort_queue(current_queue, current_queue->front+1, current_queue->rear-1);
    while (!is_empty(current_queue))
    {
	bool send_request = false;
	int queue_start = current_queue->front+1;
	int queue_end = current_queue->rear-1;
	for (int i = queue_start; i <= queue_end; ++i)
	{
	    if (dc->current_track <= current_queue->items[i].track)
	    {
		send_process_to_hdd(current_queue->items[i]);
		dequeue_tail(current_queue);
		dc->current_track = current_queue->items[i].track;
		send_request = true;
	    }
	}

	if (!send_request) dc->current_track = 0;
    }
}

void scheduleIO(Queue *q, Disk_Controler *dc)
{   
    initialize_queues();
    Queue current_queue = queues[0];
    Queue second_queue = queues[1];
    print_queue(q);
    while (!is_empty(q))
    {
	int queue_end = q->rear - 1;

	for (int i = 0; i < MAX_REQUESTS_TO_ADD_IN_QUEUE; ++i)
	{
	    enqueue(&current_queue, &q->items[queue_end - i]);
	    dequeue_tail(q);
	}
	
	process_requests(&current_queue, dc);
	
	Queue temp = current_queue;
	current_queue = second_queue;
	second_queue = temp;
	
    }
   
}
