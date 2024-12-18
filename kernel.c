#include <stdio.h>
#include <stdlib.h>

#include "kernel.h"

Process* new_process(size_t sector, size_t track, bool is_reading, bool duplicate, int next_interrupt, Mode mode, State state)
{
    Process *p = malloc(sizeof(Process));
    if (p == NULL) {
        perror("Failed to allocate memory for process");
        exit(EXIT_FAILURE);
    }
    *p = (Process){
	.sector = sector,
	.track = track,
	.is_reading = is_reading,
	.duplicate = duplicate,
	.waits_for_next_interrupt = next_interrupt,
	.mode = mode,
	.state = state
    };
    return p;
}


void generate_requests(Process* (*f)(size_t, size_t, bool, bool, int, Mode, State), IORequestNode **user_requests)
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
	int gt = gs / SECTORS_PER_TRACK;
	//generate action
	bool ga = true; // TEMPORARY action for a process.
	// In future should be random between write and read.
	bool gd = false; // duplicate
	p = f(gs, gt, ga, gd, -1, USER_MODE, NEW_PROCESS);
	printf("[SCHEDULER] Process %ld was added\n", p->sector);
	add_request(user_requests, p);
    }
}


void start_simulation()
{
    int time_worked = 0;
    int served_requests = 0;
    SchedulerType sched_t = SCHEDULER_FIFO;
    IORequestNode *user_requests = NULL;

    generate_requests(new_process, &user_requests);

    reverse_queue(&user_requests);
    
    IORequestNode *req = user_requests;

    while (served_requests != REQUESTS_NUM)
    {
        while (tick(req, &time_worked, sched_t) == 0)
	{
	    printf("\n");
	}
	printf("\n");
	if (req == NULL)
	{
	    req = user_requests;
	}
	else
	{
	    if (req->process->state == COMPLETED)
	    {
		delete_node(&user_requests, req);
		served_requests++;
		printf("\t\tserved_requests: %d\n", served_requests);
	    }
	    
	    req = req->next;
	    if (req != NULL && req->process->duplicate)
	    {
		delete_node(&user_requests, req);
		served_requests++;
	    }
	}
    }

    printf("Scheduler has nothing to do, exited with time: %d us\n", time_worked);
}


int main()
{
    //srand(NULL);
    initialize_cache();
    initialize_dc();
    //intialize_schedule_queue();
    start_simulation();
    cache_cleanup();
    return 0;
}

