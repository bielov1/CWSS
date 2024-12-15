#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "scheduler.h"

void sort_io_request_node(IORequestNode **head) {
    if (!head || !*head) return;
    
    IORequestNode *sorted = NULL;
    IORequestNode *current = *head;
    
    while (current) {
        IORequestNode *next = current->next;
        
        if (!sorted || sorted->process->sector >= current->process->sector) {
            current->next = sorted;
            current->prev = NULL;
            if (sorted) sorted->prev = current;
            sorted = current;
        } else {
            IORequestNode *temp = sorted;
            while (temp->next && temp->next->process->sector < current->process->sector) {
                temp = temp->next;
            }
            current->next = temp->next;
            current->prev = temp;
            if (temp->next) temp->next->prev = current;
            temp->next = current;
        }
        current = next;
    }
    
    *head = sorted;
}


int fifo_schedule(IORequestNode **rq)
{
    (void) rq;
    //reverse_queue(rq);
    return 0;
}

bool flook_schedule(Buffer *schedule_queue, Buffer *next_buffer)
{
    (void) schedule_queue;
    (void) next_buffer;
    return false;
}

void read_process(Process *curr_process)
{
    if (curr_process->is_reading)
    {
	printf("[SCHEDULER] Process %ld invoked read()\n", curr_process->sector);
    }
    else
    {
	printf("[SCHEDULER] Process %ld invoked write()\n", curr_process->sector);
    }
}

void tick(IORequestNode *curr_request, int *time_spent)
{
    if (curr_request == NULL)
    {
	printf("[SCHEDULER] RunQ is empty\n");
	printf("[SCHEDULER] Scheduler has nothing to do for %d us\n", 4166 - *time_spent);
	int next_interrupt = get_next_interrupt();
	printf("next interrupt is %d us\n", next_interrupt);
	if (*time_spent < next_interrupt) {
	    *time_spent = next_interrupt;
	}
	return;
    }

    
    Process *p = curr_request->process;
    
    //printf("[SCHEDULER] %d us (NEXT ITERATION)\n", *time_spent);
    switch(p->state)
    {
	case READY:
	    if (p->mode == USER_MODE)
	    {
		printf("[SCHEDULER] User mode for process %ld\n", p->sector);
		read_process(p);
		p->mode = KERNEL_MODE;
	    }
	    else if (p->mode == KERNEL_MODE)
	    {
		printf("[SCHEDULER] Kernel mode (syscall) for process %ld\n", p->sector);
		if (p->is_reading)
		{
		    printf("... worked for %d us in system, request buffer cache\n", SYSCALL_READ_TIME);
		    *time_spent += SYSCALL_READ_TIME;
		    syscall_read(p, time_spent);
		    print_device_strategy(SCHEDULER_FLOOK);
		    if (proccess_is_active_buffer(p))
		    {
			printf("[SCHEDULE] Block process %ld\n", p->sector);
			p->state = BLOCKED;
			printf("[SCHEDULER] Next interrupt from disk will be at %d us\n", p->waits_for_next_interrupt);
		    }
		}
		//else
		//{
		    //syscall_write(p);
		//}
	    }
	    break;
	case SCHEDULED:
	    if (p->is_reading)
	    {
		syscall_read(p, time_spent);
		print_device_strategy(SCHEDULER_FLOOK);
		if (proccess_is_active_buffer(p))
		{
		    printf("[SCHEDULE] Block process %ld\n", p->sector);
		    p->state = BLOCKED;
		    printf("[SCHEDULER] Next interrupt from disk will be at %d us\n", p->waits_for_next_interrupt);
		}
	    }
	    break;
	case BLOCKED:
	    if (interrupt_handler(*time_spent, p->waits_for_next_interrupt))
	    {
		printf("[SCHEDULER] Disk interrupt handler was invoked\n");
		printf("[SCHEDULER] Wake up process %ld\n", p->sector);
		p->state = WAKEUP;
	    }
	    
	    break;
	case WAKEUP:
	    complete_process();
	    p->state = COMPLETED;
	    break;
	case COMPLETED:
	    break;
    }
}
