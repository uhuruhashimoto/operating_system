//
// Created by smooth_operator on 10/8/22.
// Specs are on Page 33 of Yalnix Manual
//

#ifndef CURRENT_CHUNGUS_TRAP_HANDLERS
#define CURRENT_CHUNGUS_TRAP_HANDLERS
#include <ykernel.h>
#include "../kernel_start.h"
#include "../kernel_utils.h"
#include "trap_handlers.h"
#include "../syscalls/io_syscalls.h"
#include "../syscalls/ipc_syscalls.h"
#include "../syscalls/process_syscalls.h"
#include "../syscalls/sync_syscalls.h"
#include "../data_structures/queue.h"
#include "../debug_utils/debug.h"
#include "../data_structures/tty.h"
#define NUM_TRAP_FUNCTIONS 16

extern frame_table_struct_t *frame_table_global;
extern pcb_t* running_process;
extern pcb_t* idle_process;
extern bool is_idle;
extern queue_t* ready_queue;
extern void *trap_handler[16];
extern pte_t *region_0_page_table;
extern tty_object_t *tty_objects[NUM_TERMINALS];
extern char tty_buffer[TTY_BUFFER_SIZE]; 


// typedef void (*trap_handler_t) (UserContext* context);

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