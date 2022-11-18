#ifndef CURRENT_CHUNGUS_KERNEL_UTILS
#define CURRENT_CHUNGUS_KERNEL_UTILS
#include <ykernel.h>
#include "data_structures/pcb.h"
#include "data_structures/queue.h"

extern queue_t* ready_queue;

/*
* Top level helper to clone processes. Handles error handling, KernelContextSwitch call.
*/
int clone_process();

/*
 * Runs the next process from the queue
 *  If code==-1
 *    deletes the old process
 *  If code==0
 *    adds old process back into ready queue
 *  Otherwise
 *    the old process is blocked for some reason
 */
int install_next_from_queue(pcb_t* current_process, int code);

/*
* Top level helper to switch processes. Handles KernelContextSwitch call.
*/
int switch_between_processes(pcb_t *current_process, pcb_t *next_process);

/*
 * Clears the page table, upto the index
 */
int delete_r1_page_table(pcb_t *process, int upto_index);

/*
 * Deletes the process if no parent
 * If there is parent, triggers it
 *
 * returns ERROR on error, SUCCESS if
 */
int
delete_process(pcb_t* process, int status_code, bool do_process_switch);

/*
* Switches kernel context, deleting curr_pcb_p
*/
KernelContext *KCSwitchDelete( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p);

/*
* Switches kernel context, being careful to save special registers
*/
KernelContext *KCSwitch( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p);

/*
* Copies the kernel context from *kc_in to the *new_pcb_p, and copies
* kernel stack to unused, allocated frames. Returns kc_in.
*/
KernelContext *KCCopy( KernelContext *kc_in, void *new_pcb_p,void *not_used);

#endif