#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include "scheduler.h"
//----------------------------------------------------------------------------
void read_process(Process *curr_process)
{
    if (strcmp(curr_process->action, "READ") == 0)
    {
	printf("[SCHEDULER] Process %ld invoked read()\n", curr_process->sector);
    }
    else
    {
	printf("[SCHEDULER] Process %ld invoked write()\n", curr_process->sector);
    }
}
//----------------------------------------------------------------------------
Process* process_waits_for_interrupt = NULL;
char* last_process_action = "READ";
bool last_action_was_read = true;
Process* next_process_to_serve = NULL;

void syscall(Process *curr_process, long int* time_spent)
{    
    if (!active_buffer_exists())
    {
	move_arm_to_track(curr_process, time_spent);
	set_process_as_active_buffer(curr_process);
    }
    else
    {
	Buffer* schedule_queue = get_schedule_queue();
	schedule_process_as_buffer(schedule_queue, curr_process);
    }
}

int tick(IORequestNode** curr_request, long int *time_spent, SchedulerType sched_t)
{    
    printf("[SCHEDULER] %ld us (NEXT ITERATION)\n", *time_spent);
    if ((*curr_request) == NULL)
    {
	printf("[SCHEDULER] RunQ is empty\n");
	if (*time_spent != process_waits_for_interrupt->waits_for_next_interrupt)
	{
	    printf("[SCHEDULER] Scheduler has nothing to do for %ld us\n", process_waits_for_interrupt->waits_for_next_interrupt - *time_spent);
	    *time_spent = process_waits_for_interrupt->waits_for_next_interrupt;
	}
	return 1;
    }
    
    Process *curr_process = (*curr_request)->process;
    
    if (process_waits_for_interrupt != NULL && interrupt_handler(*time_spent, process_waits_for_interrupt->waits_for_next_interrupt))
    {
	printf("[SCHEDULER] Disk interrupt handler was invoked\n");
	printf("[SCHEDULER] Switch context to %ld\n", process_waits_for_interrupt->sector);
	process_waits_for_interrupt->state = COMPLETED;
	curr_process = process_waits_for_interrupt;
	if (sched_t != SCHEDULER_FLOOK)
	{
	    (*curr_request)->process = curr_process;
	}
	else
	{
	    IORequestNode* save_request = (*curr_request);
	    bool moved = false;
	    int push_count = 0;
	    while (save_request->process->sector != curr_process->sector)
	    {
		moved = true;
		push_count++;
		save_request = save_request->next;
	    }
	    if (moved)
	    {
		save_request->process = curr_process;
		for (int i = 0; i < push_count; ++i)
		{
		    for (int j = 0; j < push_count; ++j)
		    {
			//a b c d e f g -> a b g c d e f
			//a b d c e f g
			//a b d e c f g
			//a b d e f c g
			IORequestNode *curr_request_prev = (*curr_request)->prev; // null
			IORequestNode *curr_request_next = (*curr_request)->next; // 900
			IORequestNode *curr_request_next_next;
			
			if (curr_request_next != NULL)
			{
			    curr_request_next_next = curr_request_next->next;
			}

			
			(*curr_request)->next = curr_request_next_next;
			(*curr_request)->prev = curr_request_next;
			if (curr_request_next_next != NULL)
			{
			    curr_request_next_next->prev = (*curr_request);
			}

			if (curr_request_prev != NULL)
			{
			    curr_request_prev->next = curr_request_next;
			}

			curr_request_next->next = (*curr_request);
			curr_request_next->prev = curr_request_prev;

			//(*curr_request) = (*curr_request)->prev;

			//(*curr_request) = (*curr_request)->next;
			
		    }
		    
		    for (int k = 0; k < push_count; ++k)
		    {
			(*curr_request) = (*curr_request)->prev;
		    }
		}
		
	    }
	    else
	    {
		(*curr_request)->process = curr_process;
	    }

	    moved = false;
	    
	}
    }
    
    
    const char *curr_process_action = curr_process->action;
    
    switch(curr_process->state)
    {
	case NEW_PROCESS:
	    if (strcmp(curr_process_action, last_process_action) == 0)
	    {
		printf("[SCHEDULER] Process %ld is ready\n", curr_process->sector);
		curr_process->state = READY;
	    }
	    else
	    {
		// 4391 переключив read на write
		// після чого спрацьовує переривання, яке закінчує
		// виконнаня 4820 і новий активний буфер стає 976
		// який згенерував новий час для переривання що більше
		// минулий
		// виконує read дію. і тому не виконується контекс свіч
		// і дія з реквесто 4391 продовжує своє виконання.
		// тут виходить хуйня що last_process_action
		// стає "WRITE", хоч в активному буфері все ще
		// знаходиться процес read.
		// така хуйня, малята.
		read_process(curr_process);
		
		last_action_was_read = !last_action_was_read;
		if (!last_action_was_read) last_process_action = "WRITE";
		else last_process_action = "READ";
		curr_process->state = WAITING_FOR_INTERRUPT;
	    }
	    
	    return 0;
	    break;
	case WAITING_FOR_INTERRUPT:
	    printf("[SCHEDULER] User mode for process %ld\n", curr_process->sector);
	    if (process_waits_for_interrupt != NULL)
	    {
		printf("... worked for %ld us in user mode", process_waits_for_interrupt->waits_for_next_interrupt - *time_spent);
		*time_spent = process_waits_for_interrupt->waits_for_next_interrupt;
	    }
	    else
	    {
		printf("... worked for %d us in user mode", BEFORE_WRITING_TIME);
		*time_spent += BEFORE_WRITING_TIME;
	    }
	    curr_process->mode = KERNEL_MODE;
	    return 1;
	    break;
	case READY:
	    if (curr_process->mode == USER_MODE)
	    {
		printf("[SCHEDULER] User mode for process %ld\n", curr_process->sector);
		read_process(curr_process);
		curr_process->mode = KERNEL_MODE;
		return 0;
	    }
	    else if (curr_process->mode == KERNEL_MODE)
	    {
		printf("[SCHEDULER] Kernel mode (syscall) for process %ld\n", curr_process->sector);
		printf("... worked for %d us in system, request buffer cache\n", SYSCALL_READ_TIME);
		*time_spent += SYSCALL_READ_TIME;
		curr_process->state = BLOCKED;
		syscall(curr_process, time_spent);

		if (sched_t == SCHEDULER_FIFO)
		{
		    print_device_strategy("FIFO", sched_t);
		}
		else if (sched_t == SCHEDULER_LOOK)
		{
		    print_device_strategy("LOOK", sched_t);
		}
		else if (sched_t == SCHEDULER_FLOOK)
		{
		    print_device_strategy("FLOOK", sched_t);
		}
		printf("[SCHEDULER] Blocked process %ld\n", curr_process->sector);
		
		if (process_is_active_buffer(curr_process))
		{
		    process_waits_for_interrupt = curr_process;
		    assert(curr_process->waits_for_next_interrupt != -1);
		    printf("[SCHEDULER] Next interrupt from disk will be at %ld us\n", curr_process->waits_for_next_interrupt);
		    
		}
		
		
		return 1;
	    }
	    break;
	case BLOCKED:
	    printf("[SCHEDULER] Process %ld waited for %ld us\n", curr_process->sector, process_waits_for_interrupt->waits_for_next_interrupt - (*time_spent));
	    *time_spent = process_waits_for_interrupt->waits_for_next_interrupt;

	    return 0;
	    break;
	case COMPLETED:
	    complete_process();
	    printf("[SCHEDULER] Process %ld is completed\n", curr_process->sector);
	    
	    if (sched_t == SCHEDULER_FIFO)
	    {
		next_process_to_serve = fifo_schedule();
	    }
	    else if (sched_t == SCHEDULER_LOOK)
	    {
		next_process_to_serve = look_schedule();
	    }
	    else if (sched_t == SCHEDULER_FLOOK)
	    {
		next_process_to_serve = flook_schedule((*curr_request));
	    }

	    if (next_process_to_serve != NULL)
	    {
		
		if (next_process_to_serve->duplicate) return 1;	
	    }
	    else
	    {
		process_waits_for_interrupt->waits_for_next_interrupt = -1;
		return 1;
	    }

	    syscall(next_process_to_serve, time_spent);
	    
	    if (sched_t == SCHEDULER_FIFO)
	    {
		print_device_strategy("FIFO", sched_t);
	    }
	    else if (sched_t == SCHEDULER_LOOK)
	    {
		print_device_strategy("LOOK", sched_t);
	    }
	    else if (sched_t == SCHEDULER_FLOOK)
	    {
		print_device_strategy("FLOOK", sched_t);
	    }

	    if (process_is_active_buffer(next_process_to_serve))
	    {
		process_waits_for_interrupt = next_process_to_serve;
		printf("[SCHEDULER] Next interrupt from disk will be at %ld us\n", process_waits_for_interrupt->waits_for_next_interrupt);
	    }
	    
	    return 1;
	    break;

	}
	
    return 1;
}
//----------------------------------------------------------------------------
