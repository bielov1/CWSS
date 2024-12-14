#ifndef CACHE_LFU_H_
#define CACHE_LFU_H_

#include <stdbool.h>

#include "hdd.h"


typedef struct {   
    int counter;
    size_t sector;
    size_t track;
    bool used;
    bool active;
} Buffer;


typedef struct {
    Buffer buffers[CACHE_CAP];
} Cache;

void initialize_cache();
void move_buffer_to_front(Buffer* buffer, bool new_buffer);
bool cache_get(size_t request_sector);
void cache_print();
void cache_put(Buffer* free_buf);
void cache_cleanup();
Buffer* get_free_buffer_cache();
Buffer* find_buffer_in_cache(size_t sector);

#endif
