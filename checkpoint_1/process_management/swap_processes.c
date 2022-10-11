#include "swap_processes.h"
#include "../data_structures/pcb.h"

/*
 * This function does the actual heavy lifting of switching kernel contexts
 */
KernelContext *KCSwitch( KernelContext *kc_in,
                         void *curr pcb p,
                         void *next pcb p) {

}

/*
 * This is run when cloning a process
 *
 * KCCopy will simply copy the kernel context from *kc in into the new pcb, and copy the contents of the current
kernel stack into the frames that have been allocated for the new process’s kernel stack. However, it will then return
kc_in.
 */
KernelContext *KCCopy( KernelContext *kc_in,
                       void *new_pcb_p,
                       void *not_used) {

}

/*
 * Does the necessary stuff to swap in this process:
 *  Clears the TLB
 *
 *  Loads registers from the process's registers
 *  Sets the execution location
 *  changes the active page table
 */
int swap_in_process(pcb_t* pcb) {
  // TODO -- clear the TLB

  // TODO --
}