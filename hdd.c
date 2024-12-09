#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


#include "hdd.h"
#include "cache_lfu.h"
#include "scheduler.h"
#include "driver.h"

Process user_proc[REQUESTS_NUM];

size_t read_sector(Process p)
{
    return p.sector;
}

Process read_action(size_t sector, bool action, Mode mode, State state)
{
    return (Process) {sector, action, mode, state};
}


void generate_processes(Process (*f)(size_t, bool, Mode, State))
{
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
	Mode gm = USER_MODE;
	State gst = READY; 
	
	Process p = f(gs, ga, gm, gst);
	user_proc[i] = p;
    }
}


void simulate(SchedulerType sched_t)
{
    int time = 0;
    IORequestNode *request_queue;

    generate_processes(read_action);
    request_queue = NULL;
    
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
	add_request(&request_queue, &user_proc[i]);
    }
    
    if (sched_t == SCHEDULER_FIFO)
    {
	fifo_schedule(&request_queue);
    }
    else if (sched_t == SCHEDULER_LOOK)
    {
	//look_sort(&request_queue);
    }
    else if (sched_t == SCHEDULER_FLOOK)
    {
	flook_schedule(&request_queue, &dc, &time);
    }    
}



int main()
{
    SchedulerType sched_type = SCHEDULER_FLOOK;
    initialize_cache();
    initialize_dc();
    intialize_schedule_queue();
    simulate(sched_type);
    cache_cleanup();
    return 0;
}
