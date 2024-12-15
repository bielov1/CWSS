#include <stdio.h>

#include "kernel.h"

Process* new_process(size_t sector, bool action, int next_interrupt, Mode mode, State state)
{
    Process *p = malloc(sizeof(Process));
    if (p == NULL) {
        perror("Failed to allocate memory for process");
        exit(EXIT_FAILURE);
    }
    *p = (Process){sector, action, next_interrupt, mode, state};
    return p;
}


void generate_requests(Process* (*f)(size_t, bool, int, Mode, State), IORequestNode **user_requests)
{
    Process* p;
    int prev_gs = -1;
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	//generate sector
	int gs;

	if (prev_gs != -1 && (rand() % 100) < 30)
	{
            gs = prev_gs;
	}
	else
	{
            gs = rand() % (TOTAL_SECTORS - 1);
            prev_gs = gs;
	}
	//generate action
	bool ga = true; // TEMPORARY action for a process.
	// In future should be random between write and read.
	p = f(gs, ga, -1, USER_MODE, READY);
	printf("[SCHEDULER] Process %ld was added\n", p->sector);
	add_request(user_requests, p);
    }
}


void start_simulation()
{
    int time_worked = 0;
    int served_requests = 0;
    SchedulerType sched_t = SCHEDULER_FLOOK;
    IORequestNode *user_requests = NULL;

    generate_requests(new_process, &user_requests);
    reverse_queue(&user_requests);
    
    IORequestNode *req = user_requests;

    while (served_requests != REQUESTS_NUM)
    {
	
	for (int t = 0; t < QUANTUM_TIME; ++t)
	{
            tick(req, &time_worked);
	    if (req == NULL)
	    {
		break;
	    }
	}

	if (req == NULL)
	{
	    req = user_requests;
	}
	else
	{
	    if (req->process->state == COMPLETED) served_requests++;
	    
	    if (sched_t == SCHEDULER_FLOOK)
	    {
		req = req->next;//flook_schedule(schedule_queue);
	    }
	}
    }

    printf("Scheduler has nothing to do, exit\n");
}


int main()
{
    initialize_cache();
    initialize_dc();
    intialize_schedule_queue();
    start_simulation();
    cache_cleanup();
    return 0;
}

