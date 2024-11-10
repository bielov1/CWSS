#include <stdio.h>
#include <stdlib.h>

#include "hdd.h"
#include "cache_lfu.h"
#include "scheduler.h"



static Process user_proc[REQUESTS_NUM];
Cache *cache = NULL;
Queue q;
Disk_Controler dc;

void initialize_dc(Disk_Controler* dc)
{
    dc->current_track = 0;
}

size_t read_sector(Process p)
{
    return p.sector;
}

Process read_action(size_t sector, size_t track, bool action)
{
    return (Process) {sector, track, action};
}


void generate_request(Process (*f)(size_t, size_t, bool))
{
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	//generate sector
	int gs = rand()%TOTAL_SECTORS;
	int gt = gs/SECTORS_PER_TRACK;
	//generate action
	bool ga = true; // TEMPORARY action for a process.
	// In future should be random between write and read.
	
	Process p = f(gs, gt, ga);
	user_proc[i] = p; 
    }
}


void alloc_cache()
{
    cache = (Cache*) malloc(sizeof(Cache));
    if (cache == NULL)
    {
        fprintf(stderr, "Memory allocation for cache failed!\n");
        exit(1);
    }

    for (int i = 0; i < CACHE_CAP; ++i)
    {
        cache->buffers[i].counter = -1;
        cache->buffers[i].sector = -1;
        cache->buffers[i].used = false;
    }
}

#define SCHEDULEIO_FIFO  0
#define SCHEDULEIO_LOOK  1
#define SCHEDULEIO_FLOOK 2

#define SCHEDULEIO_IMPL SCHEDULEIO_LOOK

#if SCHEDULEIO_IMPL == SCHEDULEIO_FIFO
#include "io_fifo.c"
#elif SCHEDULEIO_IMPL == SCHEDULEIO_LOOK
#include "io_look.c"
#elif SCHEDULEIO_IMPL == SCHEDULEIO_FLOOK
#include "io_flook.c"
#else
#error "Unknown io implementation"
#endif

void send_process_to_hdd(Process p)
{
    size_t sect = 0;
    Buffer free_buffer;
    if (!cache_get(cache, p, &sect))
    {
	//perform expensive operation
	sect = read_sector(p);	
	cache_put(cache, &free_buffer, sect);
    }
}

int main()
{
    generate_request(read_action);
    alloc_cache();
    initializeQueue(&q);
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	printf("[SCHEDULER] Process %d was added.\n", i);
	enqueue(&q, &user_proc[i]);
    }
    initialize_dc(&dc);
    scheduleIO(&q, &dc);
    cache_cleanup(cache);
    return 0;
}
