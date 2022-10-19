//
// Created by smooth_operator on 10/8/22.
// Specs are on Page 33 of Yalnix Manual
//

#ifndef CURRENT_CHUNGUS_TRAP_HANDLERS
#define CURRENT_CHUNGUS_TRAP_HANDLERS

#include <hardware.h>

typedef void (*trap_handler) (UserContext* context);

/*
 * Handle traps to the kernel
 */
void handle_trap_kernel(UserContext* context);

/*
 * Handle traps to clock -- starts the next process
 */
void handle_trap_clock(UserContext* context);

/*
 * Abort the current user process
 */
void handle_trap_illegal(UserContext* context);

/*
 * Enlarges the user memory if it's an implicit request for more memory
 * otherwise kills the process
 */
void handle_trap_memory(UserContext* context);

/*
 * Aborts current user process
 */
void handle_trap_math(UserContext* context);

/*
 * Read a line from a terminal
 */
void handle_trap_tty_receive(UserContext* context);

/*
 * Write a line to a terminal
 */
void handle_trap_tty_transmit(UserContext* context);

/*
 * Handles all other traps
 */
void handle_trap_unhandled(UserContext* context);

#endif //CURRENT_CHUNGUS_TRAP_HANDLERS