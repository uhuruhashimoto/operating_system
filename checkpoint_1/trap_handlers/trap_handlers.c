#include <ykernel.h>
#include "../kernel_start.h"
#include "../kernel_utils.h"
#include "trap_handlers.h"
#include "../syscalls/io_syscalls.h"
#include "../syscalls/ipc_syscalls.h"
#include "../syscalls/process_syscalls.h"
#include "../syscalls/sync_syscalls.h"
#include "../data_structures/queue.h"

/*
 * Handle traps to the kernel
 */
void handle_trap_kernel(UserContext* context) {
  int trap_type = context->code;
  TracePrintf(1, "Handling kernel trap with code %x\n", trap_type);

  // switch based on the trap type
  switch (trap_type) {
    // process syscalls
    case YALNIX_FORK:
      handle_Fork();
      break;
    case YALNIX_EXEC:
      handle_Exec((char *)context->regs[0], (char **) context->regs[1]); //TODO cast args as temporary solution
      break;
    case YALNIX_EXIT:
      handle_Exit(context->regs[0]);
      break;
    case YALNIX_WAIT:
      handle_Wait((int *)context->regs[0]); //TODO cast args as temporary solution
      break;
    case YALNIX_GETPID:
      handle_GetPid();
      break;
    case YALNIX_BRK:
      handle_Brk((void *)context->regs[0]); //TODO cast args as temporary solution
      break;
    case YALNIX_DELAY:
      handle_Delay(context->regs[0]);
      break;

    // TTY Syscalls
    case YALNIX_TTY_READ:
//      handle_TtyRead(context->regs[0], context->regs[1], context->regs[2]);
      break;
    case YALNIX_TTY_WRITE:
//      handle_TtyWrite(context->regs[0], context->regs[1], context->regs[2]);
      break;

    // TODO -- what are YALNIX_REGISTER etc?
    // TODO -- what are YALNIX_READ_SECTOR etc?

    // IPC
    case YALNIX_PIPE_INIT:
//      handle_PipeInit(context->regs[0], context->regs[1]);
      break;
    case YALNIX_PIPE_READ:
//      handle_PipeRead(context->regs[0]);
      break;
    case YALNIX_PIPE_WRITE:
//      handle_PipeWrite(context->regs[0], context->regs[1], context->regs[2]);
      break;

    // NOP
    case YALNIX_NOP:
      // do nothing!
      break;

    // TODO -- semaphore stuff?

    case YALNIX_LOCK_INIT:
//      handle_LockInit(context->regs[0]);
      break;
    case YALNIX_LOCK_ACQUIRE:
//      handle_Acquire(context->regs[0]);
      break;
    case YALNIX_LOCK_RELEASE:
//      handle_Release(context->regs[0]);
      break;
    case YALNIX_CVAR_INIT:
//      handle_CvarInit(context->regs[0]);
      break;
    case YALNIX_CVAR_SIGNAL:
//      handle_CvarSignal(context->regs[0]);
      break;
    case YALNIX_CVAR_BROADCAST:
//      handle_CvarBroadcast(context->regs[0]);
      break;
    case YALNIX_CVAR_WAIT:
//      handle_CvarWait(context->regs[0], context->regs[1]);
      break;
    // TODO -- YALNIX_RECLAIM

    // TODO -- YALNIX_ABORT
    // TODO -- YALNIX_BOOT
  }

  // context->regs[0] = return_val
}

/*
 * Handle traps to clock -- starts the next process in the ready queue
 */
void handle_trap_clock(UserContext* context) {
  TracePrintf(1, "Our kernel hit the clock trap\n");

  if (ready_queue == NULL) {
    TracePrintf(1, "NULL READY QUEUE\n");
    return;
  }

  // TODO
  // handle Delay:
  //  go through all processes in the delay data structure
  //  decrement their delays
  //  if any process gets a delay of 0, put it back into the ready queue

  pcb_t* next_process;
  // check if there is another process in the ready queue
  if (is_empty(ready_queue)) {
    // if not, swap in the idle pcb and put the old pcb in the ready queue
    next_process = idle_process;
    is_idle = true;
    add_to_queue(ready_queue, running_process);
  }
  else {
    // if so, swap in the next process in the ready queue
    next_process = remove_from_queue(ready_queue);
    if (is_idle) {
      is_idle = false;
    }
    // we add the valid process back into the ready queue
    else {
      add_to_queue(ready_queue, running_process);
    }
  }

  // saves the current user context in the old pcb
  // clears the TLB
  switch_between_processes(running_process, next_process);
  // TODO -- how do we actually load the new kernel context ...?
}

/*
 * Handles all other traps
 */
void handle_trap_unhandled(UserContext* context) {
  TracePrintf(1, "This trap is not yet implemented\n");
  // TODO -- log something (maybe trap id?)
  // TODO -- return to user execution
}

/***************** FUTURE HANDLERS *********************/
/*
 * These will be implemented after Checkpoint 1... but we're still writing pseudocode for it!
 */


/*
 * Abort the current user process
 */
void handle_trap_illegal(UserContext* context) {
  TracePrintf(1, "This trap is not yet implemented\n");
  // abort the current process
  // get the next process from the ready queue
}

/*
 * Enlarges the user memory if it's an implicit request for more memory
 * otherwise kills the process
 */
void handle_trap_memory(UserContext* context) {
  TracePrintf(1, "This trap is not yet implemented\n");
  // implicit request for more memory -- stack, not the heap
  // check if the address being touched is one page or less away from the top of the stack
  // if so:
  //    set the brk to be a page lower
  //    return
  // otherwise:
  //  abort the process
  //  run the next process on the ready queue
}

/*
 * Aborts current user process
 */
void handle_trap_math(UserContext* context) {
  TracePrintf(1, "This trap is not yet implemented\n");
  // abort the user process
  // run the next process on the ready queue
}

/*
 * Hardware detected a new line in the terminal
 */
void handle_trap_tty_receive(UserContext* context) {
  TracePrintf(1, "This trap is not yet implemented\n");
  // int tty_id = context->code;
  // read input from terminal with TtyReceive
  // save into a terminal buffer
  // wake up waiting read processes
}

/*
 * A line being written to the terminal has completed
 */
void handle_trap_tty_transmit(UserContext* context) {
  TracePrintf(1, "This trap is not yet implemented\n");
  // int tty_id = context->code;
}

