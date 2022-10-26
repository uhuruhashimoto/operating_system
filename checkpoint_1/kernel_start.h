//
// Created by smooth_operator on 10/8/22.
//

#ifndef CURRENT_CHUNGUS_KERNEL_START_H
#define CURRENT_CHUNGUS_KERNEL_START_H

#include <ykernel.h>
#include "trap_handlers/trap_handlers.h"
#include "data_structures/pcb.h"
#include "data_structures/queue.h"
#include "data_structures/frame_table.h"

//=================== KERNEL GLOBALS ===================//
/*
* Globals that persist indefinitely for the whole kernel: 
    1. Memory storage, including the frame table and brk
    2. Process tracking, including pcbs and ready/idle/blocked queues
*/ 

int *current_kernel_brk_page;
frame_table_struct_t *frame_table_global;
pcb_t* running_process;
queue_t* ready_queue;
void *trap_handler[NUM_TRAP_FUNCTIONS];
pte_t *region_0_page_table;

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
