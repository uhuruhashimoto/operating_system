#include "kernel_start.h"
#include "trap_handlers/trap_handlers.h"
#include "data_structures/pcb.h"
#include "data_structures/queue.h"

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
  //  make sure that the page table keeps the current mapping of the kernel process space to physical memory

  // TODO -- TRAP HANDLERS
  // write base pointer of trap handlers to REG_VECTOR_BASE (how?)
  // set up trap handler array
  trap_handler[TRAP_VECTOR_SIZE];
  trap_handler[TRAP_KERNEL] = &handle_trap_kernel;
  trap_handler[TRAP_CLOCK] = &handle_trap_clock;
  trap_handler[TRAP_ILLEGAL] = &handle_trap_illegal;
  trap_handler[TRAP_MEMORY] = &handle_trap_memory;
  trap_handler[TRAP_MATH] = &handle_trap_math;
  trap_handler[TRAP_TTY_RECEIVE] = &handle_trap_tty_receive;
  trap_handler[TRAP_TTY_TRANSMIT] = &handle_trap_tty_transmit;
  trap_handler[TRAP_DISK] = &handle_trap_tty_unhandled;
  // handle all other slots in the trap vector
  for (int i=8; i<16; i++) {
    trap_handler[i] = &handle_trap_tty_unhandled;
  }

  // TODO -- allocate memory for the ready queue

  // TODO -- IDLE PCB
  // TODO -- stick the idle PCB into the running_process global

}