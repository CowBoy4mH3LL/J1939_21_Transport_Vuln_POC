//
// Created by subhojeet on 6/15/21.
//

#ifndef AMP_CHALLENGE_05_CAN_LOG_ENCRYPTION_COMMON_H
#define AMP_CHALLENGE_05_CAN_LOG_ENCRYPTION_COMMON_H

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <sys/time.h>
#include <errno.h>
#include "conf.h"

#define STATE_INITIAL 1
#define STATE_OPERATIONAL 2
#define STATE_DEFUNCT 3
#define STATE_TERMINAL 4
int state = STATE_INITIAL;

/* The CRC functions are obtained from http://home.thep.lu.se/~bjorn/crc/
 * This is siply becasue it is better this is a simplitic (possibly non-critical)
 * method and using linked headers like zlib that may not be installed on the BB
 * may not be worth it.
 */
uint32_t crc32_for_byte(uint32_t r) {
    for(int j = 0; j < 8; ++j)
        r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
    return r ^ (uint32_t)0xFF000000L;
}

void crc32(const void *data, size_t n_bytes, uint32_t* crc) {
    static uint32_t table[0x100];
    if(!*table)
        for(size_t i = 0; i < 0x100; ++i)
            table[i] = crc32_for_byte(i);
    for(size_t i = 0; i < n_bytes; ++i)
        *crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;
}

void error_local(int doexit, char *format_template_msg, ...)
{
    char prnt[256];
    memset(prnt, 0, 100);
    va_list args;
    va_start (args, format_template_msg);
    vsprintf (prnt, format_template_msg, args);
    perror(prnt);
    va_end (args);

    if (doexit == 1){exit(-1);}
}

/* ---------------------- Signaling stuff ------------------------- */

int rear = MAX_NUM_PARALLEL_TIMERS - 1;
int front = MAX_NUM_PARALLEL_TIMERS - 1;
void (*timer_handlers[MAX_NUM_PARALLEL_TIMERS]) (int sig, siginfo_t *info, void *vp); //circular queue

void install_signal(int sig, void (*handler)(int sig, siginfo_t *info, void *vp))
{
    int ret;
    struct sigaction sigact = {
            .sa_sigaction = handler,
            .sa_flags = SA_SIGINFO,
    };

    sigfillset(&sigact.sa_mask);
    ret = sigaction(sig, &sigact, NULL);
    if (ret < 0)
        error(1, "sigaction for signal %i", sig);
}

void timer_handler(int sig, siginfo_t *info, void *vp){
    if (sig == SIGALRM){
        timer_handlers[front](sig, info, vp);
        front --;
        if (front < 0){
            front = MAX_NUM_PARALLEL_TIMERS - 1;
        }
    }
}

void initiate_signals(void (*terminate_gracefully)(int sig, siginfo_t *info, void *vp)){
    //Install termination signals
    install_signal(SIGTERM, terminate_gracefully);
    install_signal(SIGINT, terminate_gracefully);

    //Install the timer handler
    install_signal(SIGALRM,timer_handler);
}

static void schedule_timer(int msec, void (* timer_handler)(int sig, siginfo_t *info, void *vp)) {
    int ret;
    struct itimerval val = {};

    val.it_value.tv_sec = msec / 1000;
    val.it_value.tv_usec = (msec % 1000) * 1000;

    timer_handlers[rear] = timer_handler;
    rear --;
    if (rear < 0){
        rear = MAX_NUM_PARALLEL_TIMERS - 1;
    }

    do {
        ret = setitimer(ITIMER_REAL, &val, NULL);
    } while ((ret < 0) && (errno == EINTR));
    if (ret < 0)
        error(1, "setitimer %i msec", msec);
}

/* ---------------------- End of Signaling stuff ------------------------- */


#endif //AMP_CHALLENGE_05_CAN_LOG_ENCRYPTION_COMMON_H
