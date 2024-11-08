#!/bin/bash

cc -Wall -Wextra -ggdb -o hdd hdd.c cache_lfu.c scheduler.c && ./hdd
