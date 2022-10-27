#ifndef CURRENT_CHUNGUS_KERNEL_UTILS
#define CURRENT_CHUNGUS_KERNEL_UTILS
#include <ykernel.h>
#include "data_structures/pcb.h"
#include "data_structures/queue.h"

extern queue_t* ready_queue;

/*
* Top level helper to clone processes. Handles error handling, KernelContextSwitch call.
*/
int clone_process();

/*
 * Runs the next process from the queue
 */
int install_next_from_queue(pcb_t* current_process);

/*
* Top level helper to switch processes. Handles KernelContextSwitch call.
*/
int switch_between_processes(pcb_t *current_process, pcb_t *next_process);

/*
* Switches kernel context, being careful to save special registers
*/
KernelContext *KCSwitch( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p);

/*
* Copies the kernel context from *kc_in to the *new_pcb_p, and copies
* kernel stack to unused, allocated frames. Returns kc_in.
*/
KernelContext *KCCopy( KernelContext *kc_in, void *new_pcb_p,void *not_used);

#endif