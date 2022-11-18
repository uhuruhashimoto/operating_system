//
// Created by smooth_operator on 10/8/22.
//

#ifndef CURRENT_CHUNGUS_KERNEL_START_H
#define CURRENT_CHUNGUS_KERNEL_START_H

#include <ykernel.h>
#include "kernel_utils.h"
#include "data_structures/pcb.h"
#include "data_structures/queue.h"
#include "data_structures/frame_table.h"
#include "data_structures/pipe.h"
#include "data_structures/cvar.h"
#include "data_structures/tty.h"
#include "trap_handlers/trap_handlers.h"
#include "process_management/load_program.h"


//=================== KERNEL GLOBALS ===================//
/*
* Globals that persist indefinitely for the whole kernel: 
    1. Memory storage, including the frame table and brk
    2. Process tracking, including pcbs and ready/idle/blocked queues
    3. Pipes, TTYs, etc
*/

// TRAP VEC
extern void *trap_handler[16];

// MEMORY
extern int current_kernel_brk_page;
extern frame_table_struct_t *frame_table_global;
extern pte_t *region_0_page_table;
extern char** cmd_args_global;

// PROCESSES
extern pcb_t* running_process;
extern pcb_t* idle_process;                                           // the special idle process; use when nothing is in ready queue
extern bool is_idle;                                                  // if is_idle, we won't put the process back on the ready queue
extern queue_t* ready_queue;
extern pcb_t *delayed_processes;

// PIPES
extern pipe_t* pipes;
extern unsigned int min_possible_pipe_id;                             // the minimum pipe id that may be allocated
extern unsigned int max_pipe_id;                                      // the maximum pipe id currently being used
extern unsigned int max_possible_pipe_id;                             // the maximum pipe id that may be allocated

// LOCKS
extern lock_t* locks;
extern unsigned int min_possible_lock_id;                             // the minimum lock id that may be allocated
extern unsigned int max_lock_id;                                      // the maximum lock id currently being used
extern unsigned int max_possible_lock_id;                             // the maximum lock id that may be allocated

// CVAR
extern cvar_t* cvars;
extern unsigned int min_possible_cvar_id;                             // the minimum cvar id that may be allocated
extern unsigned int max_cvar_id;                                      // the maximum cvar id currently being used
extern unsigned int max_possible_cvar_id;                             // the maximum cvar id that may be allocated

//TERMINALS
extern tty_object_t *tty_objects[NUM_TERMINALS];                     // metadata tracking on all the terminals
extern char tty_buffer[TTY_BUFFER_SIZE];                             // the buffer for all terminal input

//=================== KERNEL FUNCTIONS =================//
/*
 * Function that runs at kernel boot time. It initializes the different memory regions,
 * updates the globals with info available at boot time (like physical mem size), 
 * turns on virtual memory, and sets up an idle PCB.
 */
void KernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *uctxt);

/*
* Set the kernel brk based on whether or not virtual memory is enabled. This will be a syscall used by malloc and other
* higher-level user/library dynamic allocation calls.
* In case of any error, the value ERROR is returned.
*/
int SetKernelBrk(void *addr);

/*
* This functions as the "user text" for our idle PCB, looping indefinitely.
*/
void DoIdle(void);

#endif //CURRENT_CHUNGUS_KERNEL_START_H
