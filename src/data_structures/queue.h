//
// Created by smooth_operator on 10/8/22.
//

#ifndef CURRENT_CHUNGUS_QUEUE
#define CURRENT_CHUNGUS_QUEUE

#include <ykernel.h>
#include "pcb.h"

typedef struct queue {
  pcb_t* head;
  pcb_t* tail;
  int size;
} queue_t;

/*
 * Creates a new queue object
 */
queue_t* create_queue();

/*
 * is the queue empty?
 */
bool is_empty(queue_t* queue);

/*
 * Adds the PCB to the back of the queue
 */
int add_to_queue(queue_t* queue, pcb_t* pcb);

/*
 * Removes a PCB from the front of the queue
 *  For use with pipes, terminals and other things we can block on
 */
pcb_t* remove_from_queue(queue_t* queue);

#endif //CURRENT_CHUNGUS_QUEUE