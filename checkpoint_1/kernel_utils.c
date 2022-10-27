#include <ykernel.h>
#include "kernel_start.h"
#include "data_structures/pcb.h"
#include "data_structures/queue.h"

/*
* This is the highest level function for creating and cloning a new process.
* Note that there is no bookkeeping required in running processes.
*/
int clone_process(pcb_t *init_pcb) {
  int rc = KernelContextSwitch(&KCCopy, (void *)init_pcb, NULL);
  if (rc != 0) {
    TracePrintf(1, "Failed to clone kernel process; exiting...\n");
    exit(rc);
  }
  return rc;
}

/*
* This is the highest level function for switching between different processes.
* Using round robin scheduling, it grabs a ready process and swaps them out.
*/
int switch_between_processes() {
  pcb_t *current_process = running_process;
  pcb_t *next_process = remove_from_queue(ready_queue);
  int rc = KernelContextSwitch(&KCSwitch, (void *)current_process, (void *)next_process);
  if (rc != 0) {
    TracePrintf(1, "Failed to switch kernel contexts; exiting...\n");
    exit(rc);
  }
  //handle bookkeeping with the running and ready processes
  pcb_t *temp_pcb = running_process;
  running_process = next_process;
  add_to_queue(ready_queue, temp_pcb);
  return 0;
}

/*
* This is our helper for kernel context switching. In it, we change out the stack, 
* and store the current stack in the current process pcb.
*/
KernelContext *KCSwitch( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p) {
  // store current KernelContext in current pcb
  pcb_t *curr_pcb = (pcb_t  *)curr_pcb_p;
  pcb_t *next_pcb = (pcb_t *)next_pcb_p;
  memcpy(kc_in, curr_pcb->kctxt, sizeof(KernelContext));

  //store stack
  int num_stack_pages = KERNEL_STACK_MAXSIZE >> PAGESHIFT;
  for (int i=0; i<num_stack_pages; i++) {
    int stack_page_ind = (KERNEL_STACK_BASE >> PAGESHIFT) + i;
    memcpy(&region_0_page_table[stack_page_ind], curr_pcb->kernel_stack[i], sizeof(pte_t));
  } 
  //change region 0 stack mappings to those in new pcb
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_KSTACK);
  for (int i=0; i<num_stack_pages; i++) {
    int stack_page_ind = (KERNEL_STACK_BASE >> PAGESHIFT) + i;
    memcpy(next_pcb->kernel_stack[i], &region_0_page_table[stack_page_ind], sizeof(pte_t));
  } 

  return next_pcb->kctxt;
}

/*
* This is our copy helper for cloning. It copies stack and kctxt into the new pcb.
* into space already allocated by caller.
*/
KernelContext *KCCopy( KernelContext *kc_in, void *new_pcb_p,void *not_used) {
  //copy current KernelContext into initPCB
  pcb_t *init_pcb = (pcb_t  *)new_pcb_p;
  memcpy(kc_in, init_pcb->kctxt, sizeof(KernelContext));
  //copy current kernel stack into new kstack frames in initPCB
  int num_stack_pages = KERNEL_STACK_MAXSIZE >> PAGESHIFT;
  for (int i=0; i<num_stack_pages; i++) {
    int stack_page_ind = (KERNEL_STACK_BASE >> PAGESHIFT) + i;
    int new_frame = get_free_frame(frame_table_global->frame_table, frame_table_global->frame_table_size, 0);
    // use the page below the stack as a buffer to write stack pages into frames
    pte_t *bufpage = (pte_t *) KERNEL_STACK_BASE;
    bufpage->valid = 1;
    bufpage->prot = (PROT_READ | PROT_WRITE);
    bufpage->pfn = new_frame;
    //copy stack page into a new frame
    memcpy(&region_0_page_table[stack_page_ind], bufpage, sizeof(pte_t));
    //now copy that page (and associated frame) into the pcb
    memcpy(bufpage, init_pcb->kernel_stack[i], sizeof(pte_t));
  }
  // flush TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_KSTACK);
  // return pointer to the kernel context in the pcb
  return init_pcb->kctxt;
}