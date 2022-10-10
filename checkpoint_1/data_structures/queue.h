//
// Created by smooth_operator on 10/8/22.
//

#ifndef CURRENT_CHUNGUS_CIRCULAR_QUEUE
#define CURRENT_CHUNGUS_CIRCULAR_QUEUE

#include "pcb.h"

/*
 * Adds the PCB to the back of the ready queue
 */
int add_to_ready_queue(pcb_t* pcb);

/*
 * Removes a PCB from the front of the ready queue
 */
pcb_t* remove_from_ready_queue();


#endif //CURRENT_CHUNGUS_CIRCULAR_QUEUE