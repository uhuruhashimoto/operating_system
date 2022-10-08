#include "kernel_start.h"

/*
 * Behavior:
 *  Set up virtual memory
 *  Set up trap handlers
 *  Instantiate an idlePCB
 */
void KernelStart(char *cmd args[],
                 unsigned int pmem size,
                 UserContext *uctxt) {
// TODO -- VIRTUAL MEMORY

// TODO -- TRAP HANDLERS
// write base point of trap handlers to REG_VECTOR_BASE

// TODO -- IDLE PCB

}