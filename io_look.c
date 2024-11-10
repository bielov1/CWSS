#include "hdd.h"

void quick_sort_queue(int l, int r, bool reverse)
{
    int i = l;
    int j = r;

    Process w;
    
    Process x = q.items[(l+r)/2];
    while (i < j)
    {
	while (q.items[i].sector < x.sector) i++;
        while (x.sector < q.items[j].sector) j--;
     	if (i <= j)
	{
	    w = q.items[i];
	    q.items[i] = q.items[j];
	    q.items[j] = w;
	    i++;
	    j--;
	     
	}
    }
    if (l < j) quick_sort_queue(l, j, reverse);
    if (i < r) quick_sort_queue(i, r, reverse);
}

void scheduleIO(Queue *q, Disk_Controler *dc)
{
    // start point
    bool reverse = false;
    int requests_count = 0;
    int end = q->rear - 1;

    quick_sort_queue(q->front + 1, end, reverse);

    while (!is_empty(q))
    {
	bool found_request = false;
	int queue_start = q->front + 1;
	int queue_end = q->rear;
	for (int i = queue_start; i < queue_end; ++i)
        {
            if (dc->current_track == q->items[i].track)
            {
                send_process_to_hdd(q->items[i]);
                if (reverse)
		{
		    dequeue_tail(q);
		}
		else
		{
		    dequeue_head(q);
		}
                requests_count++;
                found_request = true;

		// limit check for current track to process requests that is in the same track.
                if (requests_count >= MAX_REQUESTS_PER_TRACK)
                {
                    break;
                }
            }
        }
	
	if (!found_request || requests_count >= MAX_REQUESTS_PER_TRACK)
        {
            requests_count = 0; // Оновлення лічильника запитів

            // Зміна напряму, якщо немає запитів у поточному напрямку
            if (reverse)
            {   
                dc->current_track = q->items[q->front + 1].track;
                reverse = false;
            }
            else
            {
		dc->current_track = q->items[q->rear - 1].track;
                reverse = true;
            }
        }
    }
    
}
