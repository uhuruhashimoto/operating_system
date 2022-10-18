#include <ykernel.h>
#include "pcb.h"
#include "queue.h"

/*
* basic setter: create pcb with context/page table info, initializing exit and queue info to 
* reasonable defaults. 
*/
pcb_t *create_pcb(
  pte_t *kernel_stack, 
  pte_t *region_1_page_table, 
  int page_table_reg_1_size, 
  UserContext uctxt, 
  KernelContext kctxt
) {
  pcb_t pcb = malloc(sizeof(pcb_t));
  pcb -> kernel_stack = kernel_stack;
  pcb -> region_1_page_table = malloc(sizeof(pte_t) * page_table_reg_1_size);
  pcb -> kctxt = kctxt;
  pcb -> uctxt = uctxt;
  //Defaults
  pcb -> hasExited = false;
  pcb -> rc = 0;
  pcb -> next_pcb = NULL;
  pcb -> prev_pcb = NULL;
  pcb -> children = NULL;
  pcb -> num_children = 0;
  pcb -> parent = NULL;
  return &pcb;
}

/*
 * Adds the PCB to the back of the ready queue
 */
int add_to_ready_queue(pcb_t* pcb)
{
  // TODO -- find the back of the queue, add an element to it
}

/*
 * Removes a PCB from the front of the ready queue
 */
int remove_from_ready_queue(pcb_t* pcb)
{
  // TODO -- find the front of the ready queue, remove and return that item
}