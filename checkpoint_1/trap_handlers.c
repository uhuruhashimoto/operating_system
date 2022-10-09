#include "trap_handlers.h"

/*
 * Handle traps to the kernel
 */
void handle_trap_kernel(uswr_context* context) {
  int trap_type = context->code;


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
