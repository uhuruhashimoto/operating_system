#include <ykernel.h>
#include "kernel_start.h"
#include "data_structures/pcb.h"
#include "data_structures/queue.h"

extern queue_t* ready_queue;

/*
* This is the highest level function for creating and cloning a new process.
* Note that there is no bookkeeping required in running processes.
*/
int clone_process(pcb_t *new_pcb) {
  int rc = KernelContextSwitch(&KCCopy, (void *)new_pcb, NULL);
  if (rc != 0) {
    TracePrintf(1, "Failed to clone kernel process; exiting...\n");
    Halt();
  }
  return rc;
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
* This is our helper for kernel context switching. In it, we change out the stack, 
* and store the current stack in the current process pcb.
*/
KernelContext *KCSwitch( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p) {
  // store current KernelContext in current pcb
  pcb_t *curr_pcb = (pcb_t  *)curr_pcb_p;
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