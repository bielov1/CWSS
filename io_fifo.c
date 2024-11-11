#include "hdd.h"

void scheduleIO(Queue *q, Disk_Controler *dc)
{
    (void) dc;
    Process p;
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	p = q->items[q->front];
	dequeue_head(q);
	send_process_to_hdd(p);
    }
}
