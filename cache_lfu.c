#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "cache_lfu.h"

Cache *cache = NULL;
int active_buffer_index_in_cache = -1;

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
        cache->buffers[i].process = (Process){0};
        cache->buffers[i].used = false;
        cache->buffers[i].active = false;
    }
}

void move_buffer_to_front(Buffer* buffer, bool new_buffer)
{
    if (!new_buffer && active_buffer_index_in_cache >= LEFT_SEGMENT)
    {
	buffer->counter++;
    }
    
    for (int j = active_buffer_index_in_cache; j > 0; --j)
    {
	Buffer temp = cache->buffers[j-1];
	cache->buffers[j-1] = cache->buffers[j];
	cache->buffers[j] = temp;
    }

}

bool cache_get(size_t request_sector)
{
    for (int i = 0; i < CACHE_CAP; ++i)
    {
        if (cache->buffers[i].process.sector == request_sector && cache->buffers[i].used)
        {
	    printf("[CACHE] Buffer for sector %ld was found in cache\n", request_sector);
            return true;
        }
    }
    printf("[CACHE] Buffer for sector %ld not found in cache\n", request_sector);
    return false;
}

void cache_print()
{
    printf("[CACHE] Buffer cache LFU:\n");
    printf("\tLeft Segment  [");
    for (int i = 0; i < LEFT_SEGMENT; ++i)
    {
	if (cache->buffers[i].used)
	{
	    printf("(%ld:%ld),", cache->buffers[i].process.track, cache->buffers[i].process.sector);
	}
    }
    printf("]\n");

    printf("\tMid Segment   [");
    for (int i = LEFT_SEGMENT; i < LEFT_SEGMENT + MID_SEGMENT; ++i)
    {
	if (cache->buffers[i].used)
	{
	    printf("(%ld:%ld),", cache->buffers[i].process.track, cache->buffers[i].process.sector);
	}
    }
    printf("]\n");

    printf("\tRight Segment [");
    for (int i = LEFT_SEGMENT + MID_SEGMENT; i < LEFT_SEGMENT + MID_SEGMENT + RIGHT_SEGMENT; ++i)
    {
	if (cache->buffers[i].used)
	{
	    printf("(%ld:%ld),", cache->buffers[i].process.track, cache->buffers[i].process.sector);
	}
    }
    printf("]\n");
}

void cache_put(Buffer* active_buffer)
{
    printf("[CACHE] Buffer (%ld:%ld) was added to cache\n", active_buffer->process.track, active_buffer->process.sector);
    for (int i = 0; i < CACHE_CAP; ++i)
    {
        if (!cache->buffers[i].used)
        {
	    active_buffer_index_in_cache = i;
	    cache->buffers[active_buffer_index_in_cache] = *active_buffer;
	    move_buffer_to_front(active_buffer, true);
	    return;
        }
    }

    int start_of_right_segment = LEFT_SEGMENT + MID_SEGMENT;
    int buffer_min_count = INT_MAX;
    int buffer_min_index = -1;

    for (int i = start_of_right_segment; i < CACHE_CAP; ++i)
    {
        if (cache->buffers[i].counter <= buffer_min_count)
        {
            buffer_min_count = cache->buffers[i].counter;
            buffer_min_index = i;
        }
    }

    
    active_buffer_index_in_cache = buffer_min_index;
    cache->buffers[active_buffer_index_in_cache] = *active_buffer;
    move_buffer_to_front(active_buffer, true);
    
}

void cache_cleanup()
{
    return;
}

bool find_buffer_in_cache(Buffer *buf)
{
    if (cache_get(buf->process.sector)) {
        return true;
    }
    
    return false;    
}
