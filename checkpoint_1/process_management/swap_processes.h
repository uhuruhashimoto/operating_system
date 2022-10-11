//
// Created by smooth_operator on 10/8/22.
//

#ifndef CURRENT_CHUNGUS_SWAP_PROCESSES
#define CURRENT_CHUNGUS_SWAP_PROCESSES

#include "../data_structures/pcb.h"

/*
 * Does the necessary stuff to swap in this process:
 *  Clears the TLB
 *  Loads registers from the process's registers
 *  Sets the execution location
 *  changes the active page table
 */
int swap_in_process(pcb_t* pcb);

#endif //CURRENT_CHUNGUS_SWAP_PROCESSES