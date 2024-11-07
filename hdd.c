#include <stdio.h>
#include <stdlib.h>

#include "hdd.h"
#include "cache_lfu.h"



static Process user_proc[REQUESTS_NUM];
Buffer free_buffers[CACHE_CAP];
Cache *cache = NULL;

size_t read_sector(Process p)
{
    return p.sector;
}

Process read_action(int sector, bool action)
{
    return (Process) {sector, action};
}


void generate_request(Process (*f)(int, bool))
{
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	//generate sector
	int gs = (rand()%REQUESTS_NUM)*TOTAL_SECTORS/REQUESTS_NUM;
	printf("-------------------------\n");
	printf("gs: %d\n", gs);
	printf("-------------------------\n");
	//generate action
	bool ga = true; // TEMPORARY action for a process.
	// In future should be random between write and read.
	
	Process p = f(gs, ga);
	user_proc[i] = p; 
    }
}

bool get_free_buffer(Buffer* free_buf)
{
    for (int i = 0; i < CACHE_CAP; ++i)
    {
	if (!free_buffers[i].used)
	{
	    *free_buf = free_buffers[i];
	    return true;
	}
    }
    return false;
}

int send_process_to_hdd(Process p)
{
    size_t sect = 0;
    Buffer free_buffer;
    
    if (!cache_get(cache, p, &sect))
    {
	//perform expensive operation
	sect = read_sector(p);
	if (!get_free_buffer(&free_buffer))
	{
	    printf("[INFO] No free buffers");
	}
	
	cache_put(cache, &free_buffer, sect);
    }

    //printf("%zu\n", sect);
    
    return 1;
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
        cache->buffers[i].counter = 0;
        cache->buffers[i].sector = 0;
        cache->buffers[i].used = false;
    }
}

int main()
{
    generate_request(read_action);
    
    alloc_cache();
    printf("Kernel mode\n");
    
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	send_process_to_hdd(user_proc[i]);	
    }
    
    cache_cleanup(cache);
    return 0;
}
