/*
 * circular_buffer.h
 *
 *  Created on: Sep 30, 2021
 *      Author: Sylwester Dziedziuch
 *
 * Definitions for a non intrusive type erased circular buffer.
 */

#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_

#include <stdbool.h>
#include <pthread.h>

#define BUFF_SIZE 1024

struct circular_buffer {
   void *buffer[BUFF_SIZE];

   void **head;
   void **tail;

   pthread_mutex_t head_lock;
   pthread_mutex_t tail_lock;
   pthread_cond_t empty_cond;
   pthread_cond_t full_cond;
};

void circular_buffer_init(struct circular_buffer **cbuff);
void circular_buffer_destroy(struct circular_buffer *cbuff, void (*elem_destroy)(void *));
bool circular_buffer_push(struct circular_buffer *cbuff, void *elem);
void * circular_buffer_pop(struct circular_buffer *cbuff);
void * circular_buffer_pop_nowait(struct circular_buffer *cbuff);

#endif /* _CIRCULAR_BUFFER_H_ */
