/*
 * collector.c
 *
 *  Created on: Oct 1, 2021
 *      Author: Sylwester Dziedziuch
 */

#include <sys/sysinfo.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "circular_buffer.h"
#include "collector.h"
#include "monitor.h"

/**
 * proc_cleanup: A callback for pthread_cleanup for closing /proc/stat file descriptor
 *
 * @param args: should fontain pointer to /proc/stat file descriptor
 */
static void proc_cleanup(void *args) {
   close(*((int *)args));
}

/**
 * collector_getdelim: get the string up to delimiter from file
 *
 * Function that returns a substring up to the specified delimiter
 * from the current position of the file specified by file descriptor.
 * It will allocate memory for the string and it he caller of this
 * function is responsible for freeing that memory.
 *
 * @param buffer: buffer containing resulting string
 * @param size: number of bytes read including the delimiter
 * @param delim: delimiter
 * @param fd: file descriptor of the file to read from
 *
 * @return number of bytes read including the delimiter
 */
static size_t collector_getdelim(char **buffer, size_t *size, char delim, int fd) {
   off_t begin = lseek(fd, 0, SEEK_CUR);
   *size = 0;
   int i = 0;
   char c;

   while(read(fd, &c, 1) && c != delim)
      (*size)++;

   if (size) {
      (*size)++;

      lseek(fd, begin, SEEK_SET);

      *buffer = (char *)calloc((*size), sizeof(char));

      while(read(fd, &c, 1) && c != delim)
         (*buffer)[i++] = c;

      (*buffer)[i] = '\0';
   }

   return *size;
}

/**
 * collector_getline: get a line from a specified file
 *
 * Gets a line from the specified file.
 * It will allocate memory for the string and it he caller of this
 * function is responsible for freeing that memory.
 *
 * @param buffer: buffer containing resulting string
 * @param size: number of bytes read including the newline character
 * @param fd: file descriptor of the file to read from
 *
 * @return number of bytes read including the newline character
 */
static size_t collector_getline(char **buffer, size_t *size, int fd) {
   return collector_getdelim(buffer, size, '\n', fd);
}

/**
 * parse_stats: function responsible for parsing CPU stats
 *
 * It will parse the /proc/stat file for CPU statistics.
 * It will read the file line by line for every CPU entry
 * and put it in elem.
 *
 * @param fd: file descriptor for /proc/stat file
 * @param elem: pointer to the cpu stats structure
 */
static void parse_stats(int fd, struct cpu_stats * elem) {
   int column = 1, nproc = get_nprocs();
   char *line = NULL, *token = NULL;
   size_t size;

   // Skip first line as it contains stats summed up for all cpus
   collector_getline(&line, &size, fd);
   free(line);

   for(int i=0; i<nproc; i++) {
      collector_getline(&line, &size, fd);

      column = 1;
      token = strtok(line, " ");

      while(token) {
         token = strtok(NULL, " ");

            switch(column) {
            case USER:
               elem->user[i] = atoi(token);
               break;
            case SYSTEM:
               elem->system[i] = atoi(token);
               break;
            case IDLE:
               elem->idle[i] = atoi(token);
               break;
            case IOWAIT:
               elem->iowait[i] = atoi(token);
               break;
            default:
               break;
            }
            column++;
      }
      free(line);
   }
}

void * collect(void * args) {
   struct circular_buffer *stat_buffer = (struct circular_buffer *)args;
   struct cpu_stats *stats;
   int fd = open("/proc/stat", O_RDONLY);

   if (fd >= 0) {
      pthread_cleanup_push(proc_cleanup, &fd);

      while(true) {
         // Start from the beginning of the file
         lseek(fd, 0, SEEK_SET);
         stats = (struct cpu_stats *)malloc(sizeof(struct cpu_stats));

         init_cpu_stats(stats, get_nprocs());
         parse_stats(fd, stats);
         circular_buffer_push(stat_buffer, stats);

         sleep(5);
      }
      pthread_cleanup_pop(1);
   }
   pthread_exit(NULL);
}
