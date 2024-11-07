#ifndef HDD_H_
#define HDD_H_

#include <stddef.h>
#include <stdbool.h>

#include "config.h"

typedef struct {   
    int counter;
    size_t sector;
    bool used;
} Buffer;

typedef struct {
    Buffer buffers[CACHE_CAP];
} Cache;

typedef struct {
    size_t sector;
    bool is_reading;
} Process;

#endif
