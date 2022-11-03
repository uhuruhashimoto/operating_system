#include <ykernel.h>
#include "pcb.h"

/*
* Allocate pcb data structures; e.g. kernel context and stack
*/
pcb_t *allocate_pcb() {
  int num_stack_pages = KERNEL_STACK_MAXSIZE >> PAGESHIFT;
  int reg_1_page_table_size = UP_TO_PAGE(VMEM_1_SIZE) >> PAGESHIFT;
  pcb_t *pcb = malloc(sizeof(pcb_t));
  pcb -> kernel_stack = malloc(num_stack_pages * sizeof(pte_t));
  pcb -> region_1_page_table = malloc(reg_1_page_table_size * sizeof(pte_t));
  pcb -> uctxt = malloc(sizeof(UserContext));
  pcb -> kctxt = malloc(sizeof(KernelContext));
  pcb->children = NULL;
  pcb->next_pcb = NULL;
  pcb->prev_pcb = NULL;
  pcb->next_sibling = NULL;
  pcb->prev_sibling = NULL;
  pcb->hasExited = false;
  pcb->waitingForChildExit = false;
  return pcb;
}

/*
* Set pcb values
*/
pcb_t *set_pcb_values(pcb_t *pcb, int pid, pte_t *region_1_page_table, UserContext *uctxt) {
  pcb -> pid = pid;
  pcb -> region_1_page_table = region_1_page_table;
  pcb -> uctxt = uctxt;
  return pcb;
}
