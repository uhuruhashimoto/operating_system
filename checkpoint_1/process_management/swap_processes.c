#include "swap_processes.h"
#include "../data_structures/pcb.h"
// TODO -- import KernelContext

/*
 * This function does the actual heavy lifting of switching kernel contexts
 * It will clear the TLB, swap register information
 */
KernelContext *KCSwitch( KernelContext *kc_in,
                         void *curr pcb p,
                         void *next pcb p) {
  // copy registers, execution loc into kc_in
  // return kc_in
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
  // copy registers, execution loc into kc_in
  // create a new copy of kc_in
  // copy frames over
  // place on the new PCB
  // return kc_in
}

/*
 * Does the necessary stuff to swap in this process:
 *  Clears the TLB
 *
 *  Loads registers from the process's registers
 *  Sets the execution location
 *  changes the active page table
 *
 *  Returns:
 *    the old pcb
 */
int swap_in_process(pcb_t* pcb) {
  // remove the running pcb

  // TODO -- check if the pcb has a NULL KernelContext
  // kc_in is the kernel context of the running process
  // if so:
  //  return ERROR if kc_in is NULL
  //  call KCCopy within KernelContextSwitch
  // otherwise:
  //  call KCSwitch KernelContextSwitch

  // put the return value of KernelContextSwitch back onto the running process
  // put the new pcb into the running state
  // write the current kernel context into registers
  // set up current page table (?)
  // return the old pcb
}