#include <stdio.h>
#include <stdlib.h>

#include "hdd.h"

bool interrupt_handler(int time_spent, int next_interrupt)
{
    if (time_spent == next_interrupt) {
        printf("[INTERRUPT] Handling interrupt at %d us\n", time_spent);
        return true;
    }
    return false;
}

int next_interrupt = -1;

void generate_interrupt(Process *p, int time_worked)
{
    p->waits_for_next_interrupt = time_worked + ROTATION_DELAY_TIME + SECTOR_ACCESS_TIME;
    next_interrupt = p->waits_for_next_interrupt;
}

int get_next_interrupt()
{
    return next_interrupt;
}
