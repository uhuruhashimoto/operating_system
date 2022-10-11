#include "trap_handlers.h"
#include "syscalls/io_syscalls.h"
#include "syscalls/ipc_syscalls.h"
#include "syscalls/process_syscalls.h"
#include "syscalls/sync_syscalls.h"

/*
 * Handle traps to the kernel
 */
void handle_trap_kernel() {
  // TODO -- get user context global
  int trap_type = context->code;

  // TODO -- get args from the registers

  // switch based on the trap type
  switch (trap_type) {

    // process syscalls
    case YALNIX_FORK:
      Fork();
      break;
    case YALNIX_EXEC:
      Exec();
      break;
    case YALNIX_EXIT:
      Exit();
      break;
    case YALNIX_WAIT:
      Wait();
      break;
    case YALNIX_GETPID:
      GetPid();
      break;
    case YALNIX_BRK:
      Brk();
      break;
    case YALNIX_DELAY:
      Delay();
      break;

    // TODO -- what other kernel syscalls do we need to handle?
    // TODO -- create a default case
  }

  context->regs[0] = return_val
}

/*
 * Handle traps to clock -- starts the next process in the ready queue
 */
void handle_trap_clock() {
  // TODO -- check if there is another process in the ready queue
  // if not, return to the running user process
  // if so, saves the current user context
  // clears the TLB
  // calls load_next_user_process
}

/*
 * Handles all other traps
 */
void handle_trap_unhandled() {
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
void handle_trap_illegal() {
  // abort the current process
  // get the next process from the ready queue
}

/*
 * Enlarges the user memory if it's an implicit request for more memory
 * otherwise kills the process
 */
void handle_trap_memory() {
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
void handle_trap_math() {
  // abort the user process
  // run the next process on the ready queue
}

/*
 * Read a line from a terminal
 */
void handle_trap_tty_receive() {
  // TODO -- get args from the registers
  TtyRead();
}

/*
 * Write a line to a terminal
 */
void handle_trap_tty_transmit() {
  // TODO -- get args from the registers
  TtyWrite();
}

