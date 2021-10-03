/*
 * monitor.h
 *
 *  Created on: Oct 1, 2021
 *      Author: Sylwester Dziedziuch
 */

#ifndef _MONITOR_H_
#define _MONITOR_H_

#define THRESHOLD 80

struct cpu_stats {
   unsigned int *user;
   unsigned int *system;
   unsigned int *idle;
   unsigned int *iowait;
};

struct monitor_args {
   struct circular_buffer *stat_buffer;
   struct circular_buffer *event_buffer;
};

void * monitor(void * args);
void init_cpu_stats(struct cpu_stats *elem, int nproc);
void free_cpu_stats(void * arg);

#endif /* _MONITOR_H_ */
