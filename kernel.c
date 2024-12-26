#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "kernel.h"

#define SETTINGS() \
    do { \
        printf("Settings:\n"); \
        printf("\ttracks_num:          %d\n", TRACKS); \
        printf("\tsectors_per_track:   %d\n", SECTORS_PER_TRACK); \
        printf("\ttrack_seek_time:     %d\n", TRACK_SEEK_TIME); \
        printf("\trotation_delay_time: %d\n", ROTATION_DELAY_TIME); \
        printf("\tsector_access_time:  %d\n", SECTOR_ACCESS_TIME); \
        printf("\tsyscall_read_time:   %d\n", SYSCALL_READ_TIME); \
        printf("\tsyscall_write_time:  %d\n", SYSCALL_WRITE_TIME); \
	printf("\tquantum_time:        %d\n", QUANTUM_TIME); \
        printf("\tbefore_writing_time: %d\n", BEFORE_WRITING_TIME); \
	printf("\tafter_reading_time:  %d\n", AFTER_READING_TIME); \
	printf("\tdisk_intr_time:      %d\n", DISK_INTR_TIME); \
	printf("\tbuffers num:         %d\n", CACHE_CAP); \
	printf("\tleft segment:        %d\n", LEFT_SEGMENT); \
	printf("\tmid segment:         %d\n", MID_SEGMENT); \
	printf("\tright segment:       %d\n", RIGHT_SEGMENT);\
    } while (0)

TotalTime total = {0};

//----------------------------------------------------------------------------
Process* new_process(size_t sector, size_t track, const char* action, bool duplicate, long int next_interrupt, long int finish, long int quantum_time,  Mode mode, State state)
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
	.quantum_time = quantum_time,
	.mode = mode,
	.state = state
    };
    return p;
}
//----------------------------------------------------------------------------
void generate_requests(Process* (*f)(size_t, size_t, const char*, bool, long int, long int, long int, Mode, State), IORequestNode **user_requests)
{
    Process* p;
    srand(time(NULL));
    for (int i = 0; i < REQUESTS_NUM; ++i)
    {
        int gs = (rand() % (TOTAL_SECTORS - 1)) + 1;
	int gt = gs / SECTORS_PER_TRACK;
	//generate action
	const char* ga = rand() % 2 == 0 ? "WRITE" : "READ"; 
	bool gd = false; // duplicate
	p = f(gs, gt, ga, gd, -1, 0, QUANTUM_TIME, USER_MODE, NEW_PROCESS);
	printf("[SCHEDULER] Process %ld was added witch action %s\n", p->sector, p->action);
	add_request(user_requests, p);
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
    SETTINGS();
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
	    
	    if (req->process->state == COMPLETED || req->process->duplicate)
	    {
		delete_node(&user_requests, req);
		served_requests++;
		printf("\t\tserved_requests: %d\n", served_requests);
	    }

	    req = req->next;
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
    return 0;
}
//
//----------------------------------------------------------------------------
//

