#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "scheduler.h"

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


bool last_operation_was_read = true;
Process* process_waits_for_interrupt = NULL;
Process* next_process_to_serve = NULL;

void syscall(Process *curr_process, long int* time_spent)
{    
    if (!active_buffer_exists())
    {
	set_process_as_active_buffer(curr_process);
	move_arm_to_track(curr_process, time_spent);
    }
    else
    {
	schedule_process_as_buffer(curr_process);
    }
    

}


int tick(IORequestNode* curr_request, long int *time_spent, SchedulerType sched_t)
{    
    printf("[SCHEDULER] %ld us (NEXT ITERATION)\n", *time_spent);
    if (curr_request == NULL)
    {
	printf("[SCHEDULER] RunQ is empty\n");
	if (*time_spent != process_waits_for_interrupt->waits_for_next_interrupt)
	{
	    printf("[SCHEDULER] Scheduler has nothing to do for %ld us\n", process_waits_for_interrupt->waits_for_next_interrupt - *time_spent);
	    *time_spent = process_waits_for_interrupt->waits_for_next_interrupt;
	}
	return 1;
    }
    
    Process *curr_process = curr_request->process;
    
    if (process_waits_for_interrupt != NULL && interrupt_handler(*time_spent, process_waits_for_interrupt->waits_for_next_interrupt))
    {
	printf("[SCHEDULER] Disk interrupt handler was invoked\n");
	printf("[SCHEDULER] Switch context to %ld\n", process_waits_for_interrupt->sector);
	process_waits_for_interrupt->state = COMPLETED;
	curr_process = process_waits_for_interrupt;
	curr_request->process = curr_process;
    }
    
    bool curr_process_is_reading = curr_process->is_reading;

    switch(curr_process->state)
    {
	case NEW_PROCESS:
	    // тут можна слідкувати за діями процесів і назначати щось
	    if (last_operation_was_read && curr_process_is_reading)
	    {
		printf("[SCHEDULER] Process %ld is ready\n", curr_process->sector);
		curr_process->state = READY;
	    }
	    else
	    {
		read_process(curr_process);
		if (process_waits_for_interrupt->waits_for_next_interrupt == -1)
		{
		    process_waits_for_interrupt->waits_for_next_interrupt = *time_spent + BEFORE_WRITING_TIME;
		}
		
		last_operation_was_read = !last_operation_was_read;
		curr_process->state = WAITING_FOR_INTERRUPT;
	    }
	    
	    return 0;
	    break;
	case WAITING_FOR_INTERRUPT:
	    printf("[SCHEDULER] User mode for process %ld\n", curr_process->sector);
	    printf("... worked for %ld us in user mode", process_waits_for_interrupt->waits_for_next_interrupt - *time_spent);

	    *time_spent = process_waits_for_interrupt->waits_for_next_interrupt;
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
		    print_device_strategy("FIFO");
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
		print_device_strategy("FIFO");
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
