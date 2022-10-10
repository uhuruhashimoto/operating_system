//
// Created by smooth_operator on 10/8/22.
//

#ifndef CURRENT_CHUNGUS_CIRCULAR_QUEUE
#define CURRENT_CHUNGUS_CIRCULAR_QUEUE

#include "pcb.h"

typedef struct queue queue_t;

/*
 * Adds the PCB to the back of the queue
 */
int add_to_queue(queue_t* queue, pcb_t* pcb);

/*
 * Removes a PCB from the front of the queue
 */
pcb_t* remove_from_queue(queue_t* queue,);


#endif //CURRENT_CHUNGUS_CIRCULAR_QUEUE