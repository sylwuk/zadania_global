/*
 * receiver.h
 *
 *  Created on: Oct 1, 2021
 *      Author: Sylwester Dziedziuch
 */

#ifndef _RECEIVER_H_
#define _RECEIVER_H_

struct cpu_event {
   int cpu_id;
   short usage;
};

void * receive(void * args);

#endif /* _RECEIVER_H_ */
