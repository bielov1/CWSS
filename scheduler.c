#include <stdio.h>
#include <stdlib.h>

#include "scheduler.h"

void initialize_queue(Queue *q)
{
    q->front = -1;
    q->rear = 0;
}


bool is_empty(Queue *q)
{
    return (q->front == q->rear - 1);
}
bool is_full(Queue *q)
{
    return (q->rear == REQUESTS_NUM);
}

void enqueue(Queue *q, Process *p)
{
    if (is_full(q))
    {
	printf("Queue is full\n");
	return;
    }

    q->items[q->rear] = *p;
    q->rear++;
}

void dequeue_head(Queue *q)
{
    if (is_empty(q))
    {
	printf("Queue is empty\n");
	return;
    }
    q->front++;
}

void dequeue_tail(Queue *q)
{
    if (is_empty(q))
    {
	printf("Queue is empty\n");
	return;
    }
    q->rear--;
}

Process* peek(Queue *q)
{
    if (is_empty(q))
    {
	printf("Queue is empty\n");
	return NULL;
    }
    return &q->items[q->front+1];
}

void print_queue(Queue *q)
{
    if (is_empty(q))
    {
	printf("Queue is empty\n");
	return;
    }

    printf("Current Queue: (");
    for (int i = q->front + 1; i < q->rear; ++i)
    {
	printf("%ld, ", q->items[i].sector);
    }
    printf(")\n");
}

int get_size(Queue *q)
{
    (void) q;
    return 10;
}
