#ifndef PROCESS_H_
#define PROCESS_H_

#include <stdbool.h>
#include <stdlib.h>

typedef enum State {
    READY,
    SCHEDULED,
    BLOCKED,
    WAKEUP,
    COMPLETED
} State;

typedef enum Mode {
    USER_MODE,
    KERNEL_MODE
} Mode;


typedef struct {
    size_t sector;
    bool is_reading;                 
    int waits_for_next_interrupt; // process doesn't wait for interrupt in case -1(unblocked)
                                  // process is waiting for interrupt in case > -1(blocked)
    Mode mode;                    
    State state;
} Process;


typedef struct IORequestNodeStruct {
    Process* process;
    struct IORequestNodeStruct *prev;
    struct IORequestNodeStruct *next;
} IORequestNode;



#endif
