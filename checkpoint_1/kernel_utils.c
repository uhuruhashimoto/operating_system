#include <ykernel.h>
#include "kernel_start.h"
#include "data_structures/pcb.h"
#include "data_structures/queue.h"
#include "debug_utils/debug.h"

extern frame_table_struct_t *frame_table_global;
extern pcb_t* running_process;
extern pcb_t* idle_process;                                           // the special idle process; use when nothing is in ready queue
extern bool is_idle;                                                  // if is_idle, we won't put the process back on the ready queue
extern queue_t* ready_queue;
extern void *trap_handler[16];
extern pte_t *region_0_page_table;

/*
* This is the highest level function for creating and cloning a new process.
* Note that there is no bookkeeping required in running processes.
*/
int clone_process(pcb_t *new_pcb) {
  print_reg_1_page_table(new_pcb, 5, "PRE SWITCH CLONE UTILITY");
  print_reg_1_page_table_contents(new_pcb, 5, "PRE SWITCH CLONE UTILITY");
  int rc = KernelContextSwitch(&KCCopy, (void *)new_pcb, NULL);
  if (rc != 0) {
    TracePrintf(1, "Failed to clone kernel process; exiting...\n");
    Halt();
  }
  print_reg_1_page_table(new_pcb, 5, "IN CLONE UTILITY");
  print_reg_1_page_table_contents(new_pcb, 5, "IN CLONE UTILITY");
  return new_pcb->rc;
}

/*
 * Runs the next process from the queue
 *  If code==-1
 *    deletes the old process
 *  If code==0
 *    adds old process back into ready queue
 *  Otherwise
 *    the old process is blocked for some reason
 */
int install_next_from_queue(pcb_t* current_process, int code) {
  pcb_t* next_process;
  // check if there is another process in the ready queue
  if (is_empty(ready_queue)) {
    TracePrintf(3, "INSTALL_NEXT: Queue is empty, the next process is idle\n");
    // if not, swap in the idle pcb and put the old pcb in the ready queue
    next_process = idle_process;
    // we add the valid process back into the ready queue
    if (code == 0 && !is_idle) {
      add_to_queue(ready_queue, running_process);
    }
    is_idle = true;
  }
  else {
    TracePrintf(3, "INSTALL_NEXT: Getting next item from the queue\n");
    // if so, swap in the next process in the ready queue
    next_process = remove_from_queue(ready_queue);
    if (is_idle) {
      is_idle = false;
    }
    // we add the valid process back into the ready queue
    else if (code == 0) {
      add_to_queue(ready_queue, running_process);
    }
  }

  TracePrintf(3, "INSTALL_NEXT: ABOUT TO SWAP PROCESSES\n");
  TracePrintf(3, "INSTALL_NEXT: PID of next process: %d\n", next_process->pid);
  running_process = next_process;

  // deletes the old process and swaps in the new one
  if (code == -1) {
    KernelContextSwitch(&KCSwitchDelete, (void *)current_process, (void *)(next_process));
  }
  else {
    // saves the current user context in the old pcb
    // clears the TLB
    switch_between_processes(current_process, running_process);
  }
}

/*
* This is the highest level function for switching between different processes.
*/
int switch_between_processes(pcb_t *current_process, pcb_t *next_process) {
  // sets the R1 PT
  WriteRegister(REG_PTBR1, (int) next_process->region_1_page_table);
  // Wipes the TLB for the entire process
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

  int rc = KernelContextSwitch(&KCSwitch, (void *)current_process, (void *)next_process);
  if (rc != 0) {
    TracePrintf(1, "Failed to switch kernel contexts; exiting...\n");
    Halt();
  }

  return 0;
}

/*
 * Deletes the process if no parent
 * If there is parent, triggers it
 *
 * returns ERROR on error, SUCCESS if
 */
int
delete_process(pcb_t* process, int status_code)
{
  TracePrintf(1, "DELETE PROCESS: Attempting to delete process %d with exit code %d\n", (process->pid), status_code);
  // wipe out the page table for the process
  int region_1_page_table_size = UP_TO_PAGE(VMEM_1_SIZE) >> PAGESHIFT;
  for (int i = 0; i < region_1_page_table_size; i++) {
    process->region_1_page_table[i].valid = false;
    frame_table_global->frame_table[process->region_1_page_table[i].pfn] = 0;
  }

  free(process->region_1_page_table);

  // check to see if the parent is dead; if so, completely delete the PCB and switch to the next possible process
  if (process->parent == NULL || process->parent->hasExited == true) {
    // installs the next element from the queue and COMPLETELY DELETES THE CURRENT PROCESS
    install_next_from_queue(process, -1);
  }

    // otherwise:
  else {
    process->hasExited = true;
    process->rc = status_code;

    //  switch to the parent if it is waiting for exit
    if (process->parent->waitingForChildExit == true) {
      // switch back to parent, which will loop through children again
      switch_between_processes(process, process->parent);
      // THIS LINE RUNS WHEN PARENT SWITCHES TO CHILD
      int rc = KernelContextSwitch(&KCSwitchDelete, (void *)process, (void *)(process->parent));
      if (rc != 0) {
        TracePrintf(1, "Failed to delete current kernel context; exiting...\n");
        Halt();
      }
    }
  }
}

/*
* This is our helper for kernel context switching/deletion. In it, we change out the stack.
* We also free the frames and memory for the old kernel stack
*/
KernelContext *KCSwitchDelete( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p) {
  // store current KernelContext in current pcb
  pcb_t *curr_pcb = (pcb_t *)curr_pcb_p;
  pcb_t *next_pcb = (pcb_t *)next_pcb_p;
  memcpy(curr_pcb->kctxt, kc_in, sizeof(KernelContext));

  int page_table_reg_0_size = UP_TO_PAGE(VMEM_0_SIZE) >> PAGESHIFT;
  TracePrintf(5, "=====Region 0 Page Table Before Switch=====\n");
  for (int i = 0; i < page_table_reg_0_size; i++) {
    if (region_0_page_table[i].valid) {
      TracePrintf(5, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                  VMEM_0_BASE + (i << PAGESHIFT),
                  VMEM_0_BASE + ((i+1) << PAGESHIFT)-1,
                  region_0_page_table[i].valid,
                  region_0_page_table[i].pfn
      );
    }
  }

  TracePrintf(5, "=====Freeing KernelStack=====\n");
  int num_stack_pages = KERNEL_STACK_MAXSIZE >> PAGESHIFT;
  for (int i=0; i<num_stack_pages; i++) {
    int stack_page_ind = (KERNEL_STACK_BASE >> PAGESHIFT) + i;
    // free the existing R0 kernel page tables
    region_0_page_table[stack_page_ind].valid = false;
    frame_table_global->frame_table[region_0_page_table[stack_page_ind].pfn] = 0;
    TracePrintf(5, "Deleting page in PCB: Addr: %x to %x, Valid: %d, Pfn: %d\n",
                VMEM_0_BASE + (stack_page_ind << PAGESHIFT),
                VMEM_0_BASE + ((stack_page_ind+1) << PAGESHIFT)-1,
                region_0_page_table[stack_page_ind].valid,
                region_0_page_table[stack_page_ind].pfn
    );
    // change the Region 0 kernel stack mappings to those for the new PCB
    region_0_page_table[stack_page_ind] = next_pcb->kernel_stack[i];
    TracePrintf(5, "Copying page to KERNEL: Addr: %x to %x, Valid: %d, Pfn: %d\n",
                VMEM_0_BASE + (stack_page_ind << PAGESHIFT),
                VMEM_0_BASE + ((stack_page_ind+1) << PAGESHIFT)-1,
                region_0_page_table[stack_page_ind].valid,
                region_0_page_table[stack_page_ind].pfn
    );
  }

  TracePrintf(5, "=====Region 0 Page Table After Switch=====\n");
  for (int i = 0; i < page_table_reg_0_size; i++) {
    if (region_0_page_table[i].valid) {
      TracePrintf(5, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                  VMEM_0_BASE + (i << PAGESHIFT),
                  VMEM_0_BASE + ((i+1) << PAGESHIFT)-1,
                  region_0_page_table[i].valid,
                  region_0_page_table[i].pfn
      );
    }
  }

  TracePrintf(5, "=====New PCB Kernel Stack=====\n");
  for (int i=0; i<num_stack_pages; i++) {
    TracePrintf(5, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                KERNEL_STACK_BASE + (i << PAGESHIFT),
                KERNEL_STACK_BASE + ((i + 1) << PAGESHIFT) - 1,
                next_pcb->kernel_stack[i].valid,
                next_pcb->kernel_stack[i].pfn
    );
  }

  // delete the old pcb
  free(curr_pcb);
  // set the kernel stack in region 0 to the kernel stack in the new pcb
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_KSTACK);
  return next_pcb->kctxt;
}

/*
* This is our helper for kernel context switching. In it, we change out the stack, 
* and store the current stack in the current process pcb.
*/
KernelContext *KCSwitch( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p) {
  // store current KernelContext in current pcb
  pcb_t *curr_pcb = (pcb_t *)curr_pcb_p;
  pcb_t *next_pcb = (pcb_t *)next_pcb_p;
  memcpy(curr_pcb->kctxt, kc_in, sizeof(KernelContext));

  int page_table_reg_0_size = UP_TO_PAGE(VMEM_0_SIZE) >> PAGESHIFT;
  TracePrintf(5, "=====Region 0 Page Table Before Switch=====\n");
  for (int i = 0; i < page_table_reg_0_size; i++) {
    if (region_0_page_table[i].valid) {
      TracePrintf(5, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                  VMEM_0_BASE + (i << PAGESHIFT),
                  VMEM_0_BASE + ((i+1) << PAGESHIFT)-1,
                  region_0_page_table[i].valid,
                  region_0_page_table[i].pfn
      );
    }
  }

  TracePrintf(5, "=====Copying KernelStack into Region 0=====\n");
  int num_stack_pages = KERNEL_STACK_MAXSIZE >> PAGESHIFT;
  for (int i=0; i<num_stack_pages; i++) {
    int stack_page_ind = (KERNEL_STACK_BASE >> PAGESHIFT) + i;
    // store the existing Region 0 kernel stack mappings in the old PCB
    curr_pcb->kernel_stack[i] = region_0_page_table[stack_page_ind];
    TracePrintf(5, "Storing page in PCB: Addr: %x to %x, Valid: %d, Pfn: %d\n",
                VMEM_0_BASE + (stack_page_ind << PAGESHIFT),
                VMEM_0_BASE + ((stack_page_ind+1) << PAGESHIFT)-1,
                region_0_page_table[stack_page_ind].valid,
                region_0_page_table[stack_page_ind].pfn
    );
    // change the Region 0 kernel stack mappings to those for the new PCB
    region_0_page_table[stack_page_ind] = next_pcb->kernel_stack[i];
    TracePrintf(5, "Copying page to KERNEL: Addr: %x to %x, Valid: %d, Pfn: %d\n",
                VMEM_0_BASE + (stack_page_ind << PAGESHIFT),
                VMEM_0_BASE + ((stack_page_ind+1) << PAGESHIFT)-1,
                region_0_page_table[stack_page_ind].valid,
                region_0_page_table[stack_page_ind].pfn
    );
  }

  TracePrintf(5, "=====Region 0 Page Table After Switch=====\n");
  for (int i = 0; i < page_table_reg_0_size; i++) {
    if (region_0_page_table[i].valid) {
      TracePrintf(5, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                  VMEM_0_BASE + (i << PAGESHIFT),
                  VMEM_0_BASE + ((i+1) << PAGESHIFT)-1,
                  region_0_page_table[i].valid,
                  region_0_page_table[i].pfn
      );
    }
  }

  TracePrintf(5, "=====OLD PCB Kernel Stack=====\n");
  for (int i=0; i<num_stack_pages; i++) {
    TracePrintf(5, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                KERNEL_STACK_BASE + (i << PAGESHIFT),
                KERNEL_STACK_BASE + ((i+1) << PAGESHIFT)-1,
                curr_pcb->kernel_stack[i].valid,
                curr_pcb->kernel_stack[i].pfn
    );
  }

  TracePrintf(5, "=====New PCB Kernel Stack=====\n");
  for (int i=0; i<num_stack_pages; i++) {
    TracePrintf(5, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                KERNEL_STACK_BASE + (i << PAGESHIFT),
                KERNEL_STACK_BASE + ((i + 1) << PAGESHIFT) - 1,
                next_pcb->kernel_stack[i].valid,
                next_pcb->kernel_stack[i].pfn
    );
  }

  // set the kernel stack in region 0 to the kernel stack in the new pcb
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_KSTACK);
  return next_pcb->kctxt;
}

/*
* This is our copy helper for cloning. It copies stack and kctxt into the new pcb.
* into space already allocated by caller.
*/
KernelContext *KCCopy( KernelContext *kc_in, void *new_pcb_p,void *not_used) {
  //copy current KernelContext into the new PCB
  pcb_t *new_pcb = (pcb_t  *)new_pcb_p;
  new_pcb->rc = 0;
  memcpy(new_pcb->kctxt, kc_in, sizeof(KernelContext));

  int page_table_reg_0_size = UP_TO_PAGE(VMEM_0_SIZE) >> PAGESHIFT;
  TracePrintf(5, "=====Region 0 Page Table Before Clone=====\n");
  for (int i = 0; i < page_table_reg_0_size; i++) {
    if (region_0_page_table[i].valid) {
      TracePrintf(5, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                  VMEM_0_BASE + (i << PAGESHIFT),
                  VMEM_0_BASE + ((i+1) << PAGESHIFT)-1,
                  region_0_page_table[i].valid,
                  region_0_page_table[i].pfn
      );
    }
  }

  //copy current kernel stack into new kstack frames in initPCB
  int num_stack_pages = KERNEL_STACK_MAXSIZE >> PAGESHIFT;
  for (int i=0; i<num_stack_pages; i++) {
    // bufpage should be just below the stack (-1, then -2)
    int bufpage_index = (KERNEL_STACK_BASE >> PAGESHIFT) - 1 - i;
    pte_t *bufpage = &region_0_page_table[bufpage_index];

    // get the index of the stack page to copy
    int stack_page_ind = (KERNEL_STACK_BASE >> PAGESHIFT) + i;
    int new_frame = get_free_frame(frame_table_global->frame_table, frame_table_global->frame_table_size, 0);
    // use the page below the stack as a buffer to write stack pages into frames
    bufpage->valid = 1;
    bufpage->prot = (PROT_READ | PROT_WRITE);
    bufpage->pfn = new_frame;

    //copy stack page into the new frame
    TracePrintf(5, "copying %d bytes from [%p, %p] to %p\n",
                PAGESIZE,
                stack_page_ind<<PAGESHIFT,
                ((stack_page_ind+1)<<PAGESHIFT) -1,
                bufpage_index << PAGESHIFT);

    memcpy((void *)(bufpage_index << PAGESHIFT), (void *)(stack_page_ind << PAGESHIFT), PAGESIZE);

    //now copy that page (and associated frame) into the new pcb
    pte_t page;
    page.valid = 1;
    page.prot = bufpage->prot;
    page.pfn = bufpage->pfn;
    new_pcb->kernel_stack[i] = page;

    // invalidate the bufpage, so it doesn't stick around on the stack
    bufpage->valid = 0;
  }

  TracePrintf(5, "=====Region 0 Page Table After Clone=====\n");
  for (int i = 0; i < page_table_reg_0_size; i++) {
    if (region_0_page_table[i].valid) {
      TracePrintf(5, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                  VMEM_0_BASE + (i << PAGESHIFT),
                  VMEM_0_BASE + ((i+1) << PAGESHIFT)-1,
                  region_0_page_table[i].valid,
                  region_0_page_table[i].pfn
      );
    }
  }

  TracePrintf(5, "=====OLD PCB Kernel Stack=====\n");
  for (int i=0; i<num_stack_pages; i++) {
    TracePrintf(5, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                KERNEL_STACK_BASE + (i << PAGESHIFT),
                KERNEL_STACK_BASE + ((i+1) << PAGESHIFT)-1,
                running_process->kernel_stack[i].valid,
                running_process->kernel_stack[i].pfn
    );
  }

  TracePrintf(5, "=====New PCB Kernel Stack=====\n");
  for (int i=0; i<num_stack_pages; i++) {
    TracePrintf(5, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                KERNEL_STACK_BASE + (i << PAGESHIFT),
                KERNEL_STACK_BASE + ((i + 1) << PAGESHIFT) - 1,
                new_pcb->kernel_stack[i].valid,
                new_pcb->kernel_stack[i].pfn
    );
  }


  // flush TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_KSTACK);
  // return kc_in (See Page 40)
  return kc_in;
}