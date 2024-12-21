#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "kernel.h"

ProcessFinishTime processes[REQUESTS_NUM] = {0};
TotalTime total = {0};

//----------------------------------------------------------------------------
Process* new_process(size_t sector, size_t track, const char* action, bool duplicate, long int next_interrupt, long int finish, Mode mode, State state)
{
    Process *p = malloc(sizeof(Process));
    if (p == NULL) {
        perror("Failed to allocate memory for process");
        exit(EXIT_FAILURE);
    }
    *p = (Process){
	.sector = sector,
	.track = track,
	.action = action,
	.duplicate = duplicate,
	.waits_for_next_interrupt = next_interrupt,
	.finish_time = finish,
	.mode = mode,
	.state = state
    };
    return p;
}
//----------------------------------------------------------------------------
void generate_requests(Process* (*f)(size_t, size_t, const char*, bool, long int, long int, Mode, State), IORequestNode **user_requests)
{
    Process* p;
    int index = 0;
    //srand(time(NULL));
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
        int gs = rand() % (TOTAL_SECTORS - 1);
	int gt = gs / SECTORS_PER_TRACK;
	//generate action
	const char* ga = rand() % 10 == 0 ? "READ" : "WRITE"; 
	bool gd = false; // duplicate
	p = f(gs, gt, ga, gd, -1, -1, USER_MODE, NEW_PROCESS);
	printf("[SCHEDULER] Process %ld was added witch action %s\n", p->sector, p->action);
	add_request(user_requests, p);
	processes[index++].sector = p->sector;
    }
}
//----------------------------------------------------------------------------
void start_simulation()
{
    long int time_worked = 0;
    int served_requests = 0;
    SchedulerType sched_t = SCHEDULER_LOOK;
    IORequestNode *user_requests = NULL;

    generate_requests(new_process, &user_requests);

    reverse_queue(&user_requests);
    
    IORequestNode *req = user_requests;

    while (served_requests != REQUESTS_NUM)
    {
        while (tick(&req, &time_worked, sched_t) == 0)
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
	    if (req->process->state == WAITING_FOR_INTERRUPT)
	    {
		req->process->state = READY;
		req = user_requests;
		continue;
	    }
	    
	    if (req->process->state == COMPLETED)
	    {
		for (int i = 0; i < REQUESTS_NUM; ++i)
		{
		    if (processes[i].sector == req->process->sector && processes[i].used == false)
		    {
			processes[i].finish_time = req->process->finish_time;
			processes[i].used = true;
			break;
		    }
		}
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

    printf("RunQ is empty\n");
    printf("Clean cache\n");
    cache_cleanup();
    total.time = time_worked;
    printf("Scheduler has nothing to do, exited with time: %ld us\n", time_worked);
}
//
//----------------------------------------------------------------------------
//
int main()
{
    initialize_cache();
    initialize_dc();
    start_simulation();

    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	printf("process %ld has finished with time %ld\n", processes[i].sector, processes[i].finish_time);
    }

    FILE *fp;
    
    fp = fopen("/home/oda/Programming/oda/CWSS/results/fifo_200_res.txt", "w");
    if (!fp)
    {
        perror("ERROR: can't open file\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < REQUESTS_NUM; i++)
    {
        fprintf(fp, "process: %zu time:%ld\n", processes[i].sector, processes[i].finish_time);
    }

    fclose(fp);

    fp = fopen("/home/oda/Programming/oda/CWSS/results/look_total_time_200.txt", "w");
    if (!fp)
    {
        perror("ERROR: can't open file\n");
        return EXIT_FAILURE;
    }
    
    fprintf(fp, "LOOK total time:%ld\n", total.time);
    fclose(fp);
    return 0;
}
//
//----------------------------------------------------------------------------
//

