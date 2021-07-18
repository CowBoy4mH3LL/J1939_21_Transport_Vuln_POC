#ifndef AMP_CHALLENGE_05_CAN_LOG_ENCRYPTION_CONF_H
#define AMP_CHALLENGE_05_CAN_LOG_ENCRYPTION_CONF_H

/* Network config */
#define NUM_DEVS 2 //Assumption is that all senders are numbered contigously like 0,1,2,3.... and not 0, 11, 24 etc.
#define SRC 0x49

/* System user args */
#define MAX_NUM_PARALLEL_TIMERS 10
#define LOGSELF 0 //Setting this to 1 implies this logger will log messages it sends out as well


/* Transport */
//#define ALLOC_AREA 2 //1 - heap, 2 - stack; at this time stack is not supported
#define REMALLOC 0
#define HALF_CONN_TIMER 1250 //msec
#define BITWIDTH 4

#endif //AMP_CHALLENGE_05_CAN_LOG_ENCRYPTION_CONF_H
