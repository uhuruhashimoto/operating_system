//
// Created by smooth_operator on 10/8/22.
// Specs are on Page 33 of Yalnix Manual
//

#ifndef CURRENT_CHUNGUS_TRAP_HANDLERS
#define CURRENT_CHUNGUS_TRAP_HANDLERS

typedef void (*trap_handler) ();

/*
 * Handle traps to the kernel
 */
void handle_trap_kernel();

/*
 * Handle traps to clock -- starts the next process
 */
void handle_trap_clock();

/*
 * Abort the current user process
 */
void handle_trap_illegal();

/*
 * Enlarges the user memory if it's an implicit request for more memory
 * otherwise kills the process
 */
void handle_trap_memory();

/*
 * Aborts current user process
 */
void handle_trap_math();

/*
 * Read a line from a terminal
 */
void handle_trap_tty_receive();

/*
 * Write a line to a terminal
 */
void handle_trap_tty_transmit();

/*
 * Handles all other traps
 */
void handle_trap_unhandled();

#endif //CURRENT_CHUNGUS_TRAP_HANDLERS