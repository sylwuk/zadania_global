/*
 * analyzer_main.c
 *
 *  Created on: Oct 1, 2021
 *      Author: Sylwester Dziedziuch
 *
 * Program analyzing CPU utilization and printing messages
 * to standard output if any CPU utilization exceeds 80 percent.
 *
 * It spawns three threads: collector, monitor and receiver.
 * Collector is responsible for gathering CPU utilization information
 * from /proc/stat file and pushing it to circular buffer.
 * Monitor is reading from circular buffer and analyzing collected stats.
 * If any of the CPUs present on the system exceeds 80 percent utilization
 * it sends an event to the receiver thread.
 * Receiver thread is waiting for the events from monitor and prints the
 * information about the CPU that triggered an event and its utilization.
 */

#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "circular_buffer.h"
#include "collector.h"
#include "monitor.h"
#include "receiver.h"

pthread_t monit, collector, receiver;
struct circular_buffer *stat_buffer;
struct circular_buffer *event_buffer;
struct monitor_args margs;
bool exiting = false;

static void signal_handler(int sig) {
   pthread_cancel(receiver);
   pthread_cancel(monit);
   pthread_cancel(collector);
}

int main(int argc, char **argv){
   printf("Monitoring CPU utilization...\n");

   circular_buffer_init(&stat_buffer);
   circular_buffer_init(&event_buffer);

   margs.stat_buffer = stat_buffer;
   margs.event_buffer = event_buffer;

   pthread_create(&collector, NULL, &collect, stat_buffer);
   pthread_create(&monit, NULL, &monitor, &margs);
   pthread_create(&receiver, NULL, &receive, event_buffer);

   if (signal(SIGINT, signal_handler) == SIG_ERR)
      perror("SIGINT registration failed.");
   if (signal(SIGQUIT, signal_handler) == SIG_ERR)
      perror("SIGQUIT registration failed.");

   pthread_join(receiver, NULL);
   pthread_join(monit, NULL);
   pthread_join(collector, NULL);

   circular_buffer_destroy(stat_buffer, free_cpu_stats);
   circular_buffer_destroy(event_buffer, NULL);
}
