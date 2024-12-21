#include <stdio.h>
#include <stdlib.h>

#include "hdd.h"
//----------------------------------------------------------------------------
bool interrupt_handler(long int time_spent, long int next_interrupt)
{
    return time_spent == next_interrupt;
}
//----------------------------------------------------------------------------
long int next_interrupt = -1;

void generate_interrupt(Process *p, long int time_worked)
{
    p->waits_for_next_interrupt = time_worked + ROTATION_DELAY_TIME + SECTOR_ACCESS_TIME;
    next_interrupt = p->waits_for_next_interrupt;
}
//----------------------------------------------------------------------------
int get_next_interrupt()
{
    return next_interrupt;
}
//----------------------------------------------------------------------------
