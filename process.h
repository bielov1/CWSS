#ifndef PROCESS_H_
#define PROCESS_H_

#include <stdbool.h>
#include <stdlib.h>

typedef enum State {
    NEW_PROCESS,
    WAITING_FOR_INTERRUPT,
    READY,
    BLOCKED,
    COMPLETED
} State;

typedef enum Mode {
    USER_MODE,
    KERNEL_MODE
} Mode;


typedef struct {
    size_t sector;
    size_t track;
    const char* action;
    bool duplicate;
    long int waits_for_next_interrupt; // process doesn't wait for interrupt in case -1(unblocked)
    // process is waiting for interrupt in case > -1(blocked)
    long int finish_time;
    long int quantum_time;
    Mode mode;                    
    State state;
} Process;


typedef enum {
    SCHEDULER_FIFO,
    SCHEDULER_LOOK,
    SCHEDULER_FLOOK
} SchedulerType;

typedef struct IORequestNodeStruct {
    Process* process;
    struct IORequestNodeStruct *prev;
    struct IORequestNodeStruct *next;
} IORequestNode;

typedef enum SleepQueueState {
    FREE,
    CHECK,
    EXITED
} SleepQueueState;

typedef struct {
    long int time;
    Process process;
    SleepQueueState state;
    bool used;
} SleepQueue;


#endif
