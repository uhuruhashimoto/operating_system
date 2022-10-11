//
// Created by smooth_operator on 10/8/22.
//

#ifndef CURRENT_CHUNGUS_KERNEL_START_H
#define CURRENT_CHUNGUS_KERNEL_START_H

// globals for the entire kernel
pcb_t* running_process;
queue_t* ready_queue;

/********** KernelStart ***********/
/*
 * The start function for CurrentChungus
 */
void KernelStart(char *cmd args[],
                 unsigned int pmem size,
                 UserContext *uctxt);

#endif //CURRENT_CHUNGUS_KERNEL_START_H
