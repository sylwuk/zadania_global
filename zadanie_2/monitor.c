/*
 * monitor.c
 *
 *  Created on: Oct 1, 2021
 *      Author: Sylwester Dziedziuch
 */

#include <sys/sysinfo.h>
#include <pthread.h>
#include <stdlib.h>

#include "circular_buffer.h"
#include "receiver.h"
#include "monitor.h"

void init_cpu_stats(struct cpu_stats *elem, int nproc) {
   elem->user = (unsigned int *)calloc(nproc, sizeof(unsigned int));
   elem->system = (unsigned int *)calloc(nproc, sizeof(unsigned int));
   elem->idle = (unsigned int *)calloc(nproc, sizeof(unsigned int));
   elem->iowait = (unsigned int *)calloc(nproc, sizeof(unsigned int));
}

void free_cpu_stats(void * arg) {
   struct cpu_stats *elem = arg;

   free(elem->user);
   free(elem->system);
   free(elem->idle);
   free(elem->iowait);
}

static void analyze_stats(struct circular_buffer *event_buffer, struct cpu_stats *stat) {
   unsigned long active, total;
   int percent, nproc = get_nprocs();
   struct cpu_event *event = NULL;

   for(int i=0; i<nproc; i++) {
      active = stat->user[i] + stat->system[i] + stat->iowait[i];
      total = active + stat->idle[i];
      percent = (active * 100) / total;

      if (percent >= THRESHOLD) {
         event = (struct cpu_event *)malloc(sizeof(struct cpu_event));

         event->cpu_id = i;
         event->usage = percent;

         circular_buffer_push(event_buffer, event);
      }
   }
}

void * monitor(void * args)
{
   struct circular_buffer *stat_buffer = ((struct monitor_args *)args)->stat_buffer;
   struct circular_buffer *event_buffer = ((struct monitor_args *)args)->event_buffer;
   struct cpu_stats *stat = NULL;

   while((stat = circular_buffer_pop(stat_buffer))) {
      analyze_stats(event_buffer, stat);
      free_cpu_stats(stat);
      free(stat);
   }
   pthread_exit(NULL);
}
