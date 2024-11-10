#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "hdd.h"

typedef struct {
    Process items[REQUESTS_NUM];
    int front;
    int rear;
} Queue;

void initializeQueue(Queue *q);
bool is_empty(Queue *q);
bool is_full(Queue *q);
void enqueue(Queue *q, Process *p);
void dequeue_head(Queue *q);
void dequeue_tail(Queue *q);
Process* peek(Queue *q);
void print_queue(Queue *q);
int get_size(Queue *q);



#endif
