#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "cache_lfu.h"

Cache *cache = NULL;
int free_buffer_index_in_cache = -1;

void initialize_cache()
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
        cache->buffers[i].sector = -1;
	cache->buffers[i].track = -1;
        cache->buffers[i].used = false;
    }
}

void move_buffer_to_front(Buffer* buffer, bool new_buffer)
{
    if (!new_buffer && free_buffer_index_in_cache >= LEFT_SEGMENT)
    {
	buffer->counter++;
    }
    
    for (int j = free_buffer_index_in_cache; j > 0; --j)
    {
	Buffer temp = cache->buffers[j-1];
	cache->buffers[j-1] = cache->buffers[j];
	cache->buffers[j] = temp;
    }

}

int cache_get(size_t request_sector)
{
    for (int i = 0; i < CACHE_CAP; ++i)
    {
        if (cache->buffers[i].sector == request_sector)
        {
	    printf("[CACHE] Sector `%ld` was found in cache\n", request_sector);
            move_buffer_to_front(&cache->buffers[i], false);
            return 1;
        }
    }
    
    return 0;
}

void cache_print()
{
    printf("[CACHE] Buffer cache LFU:\n");
    printf("\tLeft Segment  [");
    for (int i = 0; i < LEFT_SEGMENT; ++i)
    {
	if (cache->buffers[i].used)
	{
	    printf("(%ld:%ld),", cache->buffers[i].track, cache->buffers[i].sector);
	}
    }
    printf("]\n");

    printf("\tMid Segment   [");
    for (int i = LEFT_SEGMENT; i < LEFT_SEGMENT + MID_SEGMENT; ++i)
    {
	if (cache->buffers[i].used)
	{
	    printf("(%ld:%ld),", cache->buffers[i].track, cache->buffers[i].sector);
	}
    }
    printf("]\n");

    printf("\tRight Segment [");
    for (int i = LEFT_SEGMENT + MID_SEGMENT; i < LEFT_SEGMENT + MID_SEGMENT + RIGHT_SEGMENT; ++i)
    {
	if (cache->buffers[i].used)
	{
	    printf("(%ld:%ld),", cache->buffers[i].track, cache->buffers[i].sector);
	}
    }
    printf("]\n");
}

void cache_put(Buffer* free_buf)
{
    free_buf->used = true;
    printf("[CACHE] Buffer (%ld:%ld) added to cache\n", free_buf->track, free_buf->sector);
    cache->buffers[free_buffer_index_in_cache] = *free_buf;
    move_buffer_to_front(free_buf, true);
    
}

void cache_cleanup()
{
    return;
}

Buffer get_free_buffer_cache()
{
    Buffer found_buffer;
    
    for (int i = 0; i < CACHE_CAP; ++i)
    {
	if (!cache->buffers[i].used)
	{
	    free_buffer_index_in_cache = i;
	    return cache->buffers[i];
	}
    }

    int start_of_right_segment = LEFT_SEGMENT + MID_SEGMENT;
    int buffer_min_count = INT_MAX;

    for (int i = start_of_right_segment; i < CACHE_CAP; ++i)
    {
	if (cache->buffers[i].counter <= buffer_min_count)
	{
	    free_buffer_index_in_cache = i;
	    found_buffer = cache->buffers[i];
	    buffer_min_count = cache->buffers[i].counter;
	}
    }

    return found_buffer;
    
}

Buffer request_buffer_cache(IORequestNode *active_request)
{
    if (!cache_get(active_request->process->sector))
    {
	printf("[CACHE] Buffer for sector %ld not found in cache\n", active_request->process->sector);
	printf("[CACHE] Get free buffer\n");
	Buffer free_buffer = get_free_buffer_cache();
	free_buffer.counter = 1;
	free_buffer.sector = active_request->process->sector;
	free_buffer.track = active_request->process->sector/SECTORS_PER_TRACK;
	free_buffer.used = false;

	return free_buffer; 
    }
    
}
