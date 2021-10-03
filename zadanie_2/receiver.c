/*
 * receiver.c
 *
 *  Created on: Oct 1, 2021
 *      Author: Sylwester Dziedziuch
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "circular_buffer.h"
#include "receiver.h"

void * receive(void * args) {
   struct circular_buffer *event_buffer = (struct circular_buffer *)args;
   struct cpu_event *event = NULL;

   while((event = circular_buffer_pop(event_buffer))) {
      printf("Event! CPU%d overloaded usage: %d\n", event->cpu_id, event->usage);
      free(event);
   }

   pthread_exit(NULL);
}
