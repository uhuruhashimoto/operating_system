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
#include "trap_handlers/trap_handlers.h"
#include "process_management/load_program.h"

//=================== KERNEL GLOBALS ===================//
/*
* Globals that persist indefinitely for the whole kernel: 
    1. Memory storage, including the frame table and brk
    2. Process tracking, including pcbs and ready/idle/blocked queues
*/ 

extern int *current_kernel_brk_page;
extern frame_table_struct_t *frame_table_global;
extern pcb_t* running_process;
extern pcb_t* idle_process;                                           // the special idle process; use when nothing is in ready queue
extern bool is_idle;                                                  // if is_idle, we won't put the process back on the ready queue
extern queue_t* ready_queue;
extern void *trap_handler[16];
extern pte_t *region_0_page_table;

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
