#ifndef CONFIG_H_
#define CONFIG_H_

#define REQUESTS_NUM 10
#define TRACKS 10
#define SECTORS_PER_TRACK 500
#define TOTAL_SECTORS TRACKS*SECTORS_PER_TRACK
#define CACHE_CAP 7
#define LEFT_SEGMENT 2
#define MID_SEGMENT 2
#define RIGHT_SEGMENT 3
#define MAX_REQUESTS_PER_TRACK 3
#define MAX_REQUESTS_TO_ADD_IN_QUEUE 5
#define QUANTUM_TIME 20000

#endif
