#include "hdd.h"

void scheduleIO(Queue *q)
{
    (void) q;
    return (Process) {.sector = 100, .is_reading = true};
}
