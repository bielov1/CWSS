#ifndef HDD_H_
#define HDD_H_

#include "config.h"
#include "process.h"

bool interrupt_handler(long int time_spent, long int next_interrupt);
void generate_interrupt(Process *p, long int time_worked);
int get_next_interrupt();
#endif
