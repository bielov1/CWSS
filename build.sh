#!/bin/bash

cc -Wall -Wextra -ggdb -o hdd hdd.c cache_lfu.c scheduler.c driver.c && ./hdd
rm ./hdd
