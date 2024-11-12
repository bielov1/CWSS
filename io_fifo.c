#include "hdd.h"

void scheduleIO(Queue *q, Disk_Controler *dc)
{
    (void) dc;
    int queue_start = q->front + 1;
    int queue_end = q->rear - 1;
    for (int i = queue_start; i <= queue_end; ++i)
    {
	send_process_to_hdd(q->items[i]);
	dequeue_head(q);
    }
}
