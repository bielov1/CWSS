#ifndef HDD_H_
#define HDD_H_

#include <stddef.h>
#include <stdbool.h>

#include "config.h"

typedef enum State {
    READY,
    BLOCKED,
    WAKEUP,
    FINISHED
} State;

typedef enum Mode {
    USER_MODE,
    KERNEL_MODE
} Mode;


typedef enum {
    SCHEDULER_FIFO,
    SCHEDULER_LOOK,
    SCHEDULER_FLOOK
} SchedulerType;

typedef struct {
    size_t sector;
    bool is_reading;
    Mode mode;
    State state;
} Process;

typedef struct IORequestNodeStruct {
    Process* process;
    struct IORequestNodeStruct *prev;
    struct IORequestNodeStruct *next;
} IORequestNode;


typedef struct {
    int head_pos;
    int head_direction;
} Disk_Controler;


void send_request_to_hdd(Process p);

#endif
