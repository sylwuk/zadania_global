/*
 * circular_buffer.c
 *
 *  Created on: Sep 30, 2021
 *      Author: Sylwester Dziedziuch
 *
 * Implementation of non intrusive type erased thread safe circular buffer.
 */

#include <stdlib.h>

#include "circular_buffer.h"

/**
 * buffer_cleanup: A callback for pthread_cleanup for unlocking buffer mutex
 *
 * @param args: should point to the mutex to be unlocked
 */
static void buffer_cleanup(void *args) {
   pthread_mutex_unlock((pthread_mutex_t *)args);
}

/**
 * get_next_to_push: Returns a pointer to the next element in the buffer to push to
 *
 * @param cbuff: pointer to circular buffer to push to
 *
 * @return pointer to the next element in the buffer to push to
 */
static void ** get_next_to_push(struct circular_buffer *cbuff) {
   if (cbuff->head == &(cbuff->buffer[BUFF_SIZE-1]))
      return cbuff->buffer;
   else
      return cbuff->head + 1;
}

/**
 * get_next_to_pop: Returns a pointer to the next element in the buffer to pop from
 *
 * @param cbuff: pointer to circular buffer to pop from
 *
 * @return pointer to the next element in the buffer to pop from
 */
static void ** get_next_to_pop(struct circular_buffer *cbuff) {
   if (cbuff->tail == &(cbuff->buffer[BUFF_SIZE-1]))
      return cbuff->buffer;
   else
      return cbuff->tail + 1;
}

/**
 * is_full: Checks if the buffer is full
 *
 * @param cbuff: pointer to circular buffer
 *
 * @return true if buffer is full false otherwise
 */
static bool is_full(struct circular_buffer *cbuff) {
   if (cbuff->head != NULL && cbuff->tail == get_next_to_push(cbuff))
      return true;

   return false;
}

/**
 * is_empty: Checks if the buffer is empty
 *
 * @param cbuff: pointer to circular buffer
 *
 * @return true if buffer is empty false otherwise
 */
static bool is_empty(struct circular_buffer *cbuff) {
   if (cbuff->head == cbuff->tail && *cbuff->tail == NULL)
      return true;

   return false;
}

/**
 * circular_buffer_push: pushing elements to the buffer
 *
 * Buffer needs to be initialized prior to pushing elements.
 * Will block and wait for element to be popped from the buffer
 * if the buffer is full.
 *
 * @param cbuff: pointer to circular buffer to push to
 * @param elem: pointer to the element to push to the buffer
 *
 * @return true if push succeeded false otherwise
 */
bool circular_buffer_push(struct circular_buffer *cbuff, void *elem) {
   if (pthread_mutex_lock(&cbuff->head_lock))
      return false;

   pthread_cleanup_push(buffer_cleanup, &cbuff->head_lock);

   while (is_full(cbuff)) {
      if (pthread_cond_wait(&cbuff->full_cond, &cbuff->head_lock))
         return false;
   }

   *cbuff->head = elem;

   if (!is_full(cbuff))
      cbuff->head = get_next_to_push(cbuff);

   pthread_cond_signal(&cbuff->empty_cond);
   pthread_cleanup_pop(1);

   return true;
}

/**
 * circular_buffer_pop: pop elements from the buffer
 *
 * Buffer needs to be initialized prior to trying to pop elements.
 * Will block and wait for the element to be added to the buffer
 * if the buffer is empty.
 * Element popped from the buffer needs to be freed by the caller.
 *
 * @param cbuff: pointer to circular buffer to pop from
 *
 * @return pointer to the popped element or NULL if failed to lock the mutex
 */
void * circular_buffer_pop(struct circular_buffer *cbuff) {
   void *ret = NULL;

   if (pthread_mutex_lock(&cbuff->tail_lock))
      return NULL;

   pthread_cleanup_push(buffer_cleanup, &cbuff->tail_lock);

   while(is_empty(cbuff)) {
      if (pthread_cond_wait(&cbuff->empty_cond, &cbuff->tail_lock))
         return NULL;
   }

   ret = *(cbuff->tail);
   *(cbuff->tail) = NULL;
   cbuff->tail = get_next_to_pop(cbuff);

   pthread_cond_signal(&cbuff->full_cond);
   pthread_cleanup_pop(1);

   return ret;
}

/**
 *circular_buffer_pop_nowait: pop elements from the buffer without waiting
 *
 * Buffer needs to be initialized prior to trying to pop elements.
 * Non blocking version of pop. If buffer is empty it will return NULL;
 * Element popped from the buffer needs to be freed by the caller.
 *
 * @param cbuff: pointer to circular buffer to pop from
 *
 * @return pointer to the popped element or NULL if failed to lock the mutex or empty
 */
void * circular_buffer_pop_nowait(struct circular_buffer *cbuff) {
   void *ret = NULL;

   if (pthread_mutex_lock(&cbuff->tail_lock))
      return NULL;

   if (!is_empty(cbuff)) {
      ret = *(cbuff->tail);
      *(cbuff->tail) = NULL;
      cbuff->tail = get_next_to_pop(cbuff);
   }

   pthread_cond_signal(&cbuff->full_cond);
   pthread_mutex_unlock(&cbuff->tail_lock);

   return ret;
}

/**
 * circular_buffer_init: buffer initialization function
 *
 * Function will allocate memory for the buffer, reset all the pointers
 * and initialize mutexes and conditional variables.
 *
 * @param cbuff: pointer to pointer to the buffer to be initialized
 */
void circular_buffer_init(struct circular_buffer **cbuff) {
   *cbuff = (struct circular_buffer *)malloc(sizeof(struct circular_buffer));

   for(int i=0; i<BUFF_SIZE; i++) {
      (*cbuff)->buffer[i] = NULL;
   }

   (*cbuff)->head = (*cbuff)->tail = (*cbuff)->buffer;

   pthread_mutex_init(&(*cbuff)->head_lock, NULL);
   pthread_mutex_init(&(*cbuff)->tail_lock, NULL);
   pthread_cond_init(&(*cbuff)->empty_cond, NULL);
   pthread_cond_init(&(*cbuff)->full_cond, NULL);
}

/**
 * circular_buffer_destroy:buffer cleanup function
 *
 * Caller need to make sure no threads are waiting on conditional variables.
 * It will pop all the remaining elements in the buffer and free them.
 * It will also call element cleanup function if element type has non trivial cleanup.
 *
 * @param cbuff: pointer to pointer to the buffer to be cleaned up
 * @param elem_destroy: callback to element cleanup function
 */
void circular_buffer_destroy(struct circular_buffer *cbuff, void (*elem_destroy)(void *)) {
   void *entry;

   while((entry = circular_buffer_pop_nowait(cbuff))) {
      if (elem_destroy)
         elem_destroy(entry);
      free(entry);
   }

   pthread_mutex_destroy(&cbuff->tail_lock);
   pthread_mutex_destroy(&cbuff->head_lock);
   pthread_cond_destroy(&cbuff->full_cond);
   pthread_cond_destroy(&cbuff->empty_cond);

   free(cbuff);
}
