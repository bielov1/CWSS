#ifndef CONFIG_H_
#define CONFIG_H_

#define REQUESTS_NUM 10
#define TOTAL_SECTORS TRACKS*SECTORS_PER_TRACK
#define CACHE_CAP 4
#define LEFT_SEGMENT 1
#define MID_SEGMENT 1
#define RIGHT_SEGMENT 2
#define MAX_REQUESTS_PER_TRACK 3
#define MAX_REQUESTS_TO_ADD_IN_QUEUE 5

#define TRACKS 10
#define SECTORS_PER_TRACK 500
#define QUANTUM_TIME 20000
#define ROTATION_DELAY_TIME 4000
#define SECTOR_ACCESS_TIME 16
#define TRACK_SEEK_TIME 500
#define SYSCALL_READ_TIME 150

#endif
