#include "trap_handlers.h"

/*
 * Handle traps to the kernel
 */
void handle_trap_kernel(uswr_context* context) {
  int trap_type = context->code;


  context->regs[0] = return_val
}

/*
 * Handle traps to clock -- starts the next process
 */
void handle_trap_clock();

/*
 * Handles all other traps
 */
void handle_trao_unhandled();
