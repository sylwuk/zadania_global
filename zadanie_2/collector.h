/*
 * collector.h
 *
 *  Created on: Oct 1, 2021
 *      Author: Sylwester Dziedziuch
 */

#ifndef _COLLECTOR_H_
#define _COLLECTOR_H_

enum columns {
   USER = 1,
   SYSTEM = 3,
   IDLE = 4,
   IOWAIT = 5
};

void * collect(void * args);

#endif /* _COLLECTOR_H_ */
