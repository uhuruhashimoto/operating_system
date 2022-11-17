#include <ykernel.h>
#include "pcb.h"

/*
* Allocate pcb data structures; e.g. kernel context and stack
* If we encounter an error, we fail atomically and return NULL after freeing any allocated structures.
*/
pcb_t *allocate_pcb() {
  int num_stack_pages = KERNEL_STACK_MAXSIZE >> PAGESHIFT;
  int reg_1_page_table_size = UP_TO_PAGE(VMEM_1_SIZE) >> PAGESHIFT;
  pcb_t *pcb = malloc(sizeof(pcb_t));
  if (pcb == NULL) {
    TracePrintf(1, "Failed to allocate memory for a new pcb\n");
    return NULL;
  }
  pcb -> kernel_stack = malloc(num_stack_pages * sizeof(pte_t));
  if (pcb->kernel_stack == NULL) {
    TracePrintf(1, "Failed to allocate memory for a new pcb's kernel stack\n");
    free(pcb);
    return NULL;
  }
  pcb -> region_1_page_table = malloc(reg_1_page_table_size * sizeof(pte_t));
  if (pcb->region_1_page_table == NULL) {
    TracePrintf(1, "Failed to allocate memory for a new pcb's region 1 page table\n");
    free(pcb->kernel_stack);
    free(pcb);
    return NULL;
  }
  pcb -> uctxt = malloc(sizeof(UserContext));
  if (pcb->uctxt == NULL) {
    TracePrintf(1, "Failed to allocate memory for a new pcb's user context\n");
    free(pcb->region_1_page_table);
    free(pcb->kernel_stack);
    free(pcb);
    return NULL;
  }
  pcb -> kctxt = malloc(sizeof(KernelContext));
  if (pcb->kctxt == NULL) {
    TracePrintf(1, "Failed to allocate memory for a new pcb's kernel context\n");
    free(pcb->uctxt);
    free(pcb->region_1_page_table);
    free(pcb->kernel_stack);
    free(pcb);
    return NULL;
  }

  pcb->brk_floor = 0;
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
  if (pcb == NULL) {
    TracePrintf(1, "set_pcb_values: pcb is NULL\n");
    return NULL;
  }
  pcb -> pid = pid;
  pcb -> region_1_page_table = region_1_page_table;
  pcb -> uctxt = uctxt;
  return pcb;
}
