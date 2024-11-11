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
    size_t track;
    bool is_reading;
} Process;

typedef struct {
    size_t current_track;
} Disk_Controler;

void send_process_to_hdd(Process p);

#endif
