#ifndef CACHE_LFU_H_
#define CACHE_LFU_H_

#include "config.h"
#include "hdd.h"


typedef struct {   
    int counter;
    size_t sector;
    size_t track;
    bool used;
} Buffer;


typedef struct {
    Buffer* buffers[CACHE_CAP];
} Cache;

void initialize_cache();
void move_buffer_to_front(int index, bool new_buffer);
Buffer* cache_get(size_t request_sector);
void cache_put(Buffer *free_buf);
void cache_cleanup();
Buffer* get_free_buffer_cache();
Buffer* request_buffer_cache(IORequestNode *active_request);

#endif
