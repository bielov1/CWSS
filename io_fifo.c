#include "hdd.h"

void scheduleIO(Queue *q)
{
    Process p;
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	p = q->items[q->front];
	dequeue(q);
	send_process_to_hdd(p);
    }
}
