#include <stdio.h>

#include "cache_lfu.h"
#include "hdd.h"

void move_buffer_to_front(Cache *cache, int index)
{
    if (index > LEFT_SEGMENT || index > LEFT_SEGMENT + MID_SEGMENT)
    {
	cache->buffers[index].counter++; 
    }

    for (int i = index; i > 0; --i)
    {
	Buffer temp = cache->buffers[i-1];
	cache->buffers[i-1] = cache->buffers[i];
	cache->buffers[i] = temp;
    }
}

bool cache_get(Cache *cache, Process p, size_t *sector)
{
    for (int i = 0; i < CACHE_CAP; ++i)
    {
        if (cache->buffers[i].sector == p.sector)
        {
            if (sector) *sector = cache->buffers[i].sector;
            move_buffer_to_front(cache, i);
            return true;
        }
    }
    
    return false;
}

void cache_put(Cache *cache, Buffer *free_buf, size_t sector)
{
    for (int i = 0; i < CACHE_CAP; ++i)
    {
	// TODO: implement put case when cache is full
	if (!cache->buffers[i].used)
	{
	    free_buf->counter = 1;
	    free_buf->sector = sector;
	    free_buf->used = true;
	    
	    cache->buffers[i] = *free_buf;
	    move_buffer_to_front(cache, i);
	    return;
	}
    }
}

void cache_cleanup(Cache *cache)
{
    for (int i = 0; i < CACHE_CAP; ++i)
    {
	printf("Buffer %d:\n    counter = %d\n    sector = %ld\n    used = %b\n\n", i, cache->buffers[i].counter, cache->buffers[i].sector, cache->buffers[i].used);
    }
    (void) cache;
}
