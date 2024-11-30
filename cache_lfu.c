#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "cache_lfu.h"

Cache *cache = NULL;

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
        cache->buffers[i] = (Buffer*) malloc(sizeof(Buffer));
        if (cache->buffers[i] == NULL)
        {
            fprintf(stderr, "Memory allocation for buffer %d failed!\n", i);
            exit(1);
        }

        cache->buffers[i]->counter = 0;
        cache->buffers[i]->sector = -1;
        cache->buffers[i]->used = false;
    }
}

void move_buffer_to_front(int buffer_index, bool new_buffer)
{
    if (!new_buffer && buffer_index >= LEFT_SEGMENT)
    {
	cache->buffers[buffer_index]->counter++; 
    }

    for (int i = buffer_index; i > 0; --i)
    {
	Buffer* temp = cache->buffers[i-1];
	cache->buffers[i-1] = cache->buffers[i];
	cache->buffers[i] = temp;
    }
}

Buffer* cache_get(size_t request_sector)
{
    for (int i = 0; i < CACHE_CAP; ++i)
    {
        if (cache->buffers[i]->sector == request_sector)
        {
            move_buffer_to_front(i, false);
            return cache->buffers[0];
        }
    }
    
    return NULL;
}

void cache_put(Buffer *free_buf)
{
    for (int i = 0; i < CACHE_CAP; ++i)
    {
	if (cache->buffers[i]->sector == free_buf->sector)
	{
	    cache->buffers[i]->used = true;
	    move_buffer_to_front(i, true);
	}
    }
}

void cache_cleanup()
{
    for (int i = 0; i < CACHE_CAP; ++i)
    {
	printf("Buffer %d:\n    counter = %d\n    sector = %ld\n    used = %b\n\n", i, cache->buffers[i]->counter, cache->buffers[i]->sector, cache->buffers[i]->used);
    }
}

Buffer* get_free_buffer_cache()
{
    Buffer *found_buffer;
    
    for (int i = 0; i < CACHE_CAP; ++i)
    {
	if (!cache->buffers[i]->used) return cache->buffers[i]; 
    }

    int start_of_right_segment = LEFT_SEGMENT + MID_SEGMENT;
    int buffer_min_count = INT_MAX;

    for (int i = start_of_right_segment; i < CACHE_CAP; ++i)
    {
	if (cache->buffers[i]->counter <= buffer_min_count)
	{
	    found_buffer = cache->buffers[i];
	    buffer_min_count = cache->buffers[i]->counter;
	}
    }

    return found_buffer;
    
}

Buffer* request_buffer_cache(IORequestNode *active_request)
{
    Buffer *found_buffer = cache_get(active_request->process->sector);
    if (found_buffer == NULL)
    {
	printf("[CACHE] Buffer for sector %ld not found in cache\n", active_request->process->sector);
	printf("[CACHE] Get free buffer\n");
	Buffer *free_buffer = get_free_buffer_cache();
	free_buffer->counter = 1;
	free_buffer->sector = active_request->process->sector;
	free_buffer->track = active_request->process->sector/SECTORS_PER_TRACK;
	free_buffer->used = false;

	return free_buffer; 
    }

    return found_buffer;
}
