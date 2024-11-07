#ifndef CACHE_LFU_H_
#define CACHE_LFU_H_

#include "hdd.h"

void move_buffer_to_front(Cache *cache, int index);
bool cache_get(Cache *cache, Process p, size_t *sector);
void cache_put(Cache *cache, Buffer *free_buf, size_t sector);
void cache_cleanup(Cache *cache);

#endif
