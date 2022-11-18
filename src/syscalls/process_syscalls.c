#include <ykernel.h>
#include "../kernel_start.h"
#include "../kernel_utils.h"
#include "../data_structures/pcb.h"
#include "../data_structures/queue.h"
#include "../data_structures/frame_table.h"
#include "../debug_utils/debug.h"
#include "../memory/check_memory.h"

extern frame_table_struct_t *frame_table_global;
extern pcb_t* running_process;
extern pcb_t* idle_process;                                           // the special idle process; use when nothing is in ready queue
extern bool is_idle;                                                  // if is_idle, we won't put the process back on the ready queue
extern queue_t* ready_queue;
extern void *trap_handler[16];
extern pte_t *region_0_page_table;

  /*
  * Fork the process and create a new, separate address space.
  * We give the child a unique pid and copy over the user context and region 1 page table.
  * Then we walk through the page table and copy the contents of valid pages into new frames.
  * TODO: Copy on write, eventually
  */
int handle_Fork(void)
{
  TracePrintf(1, "FORK_HANDLER: Attempting to fork a process based on the running process\n");

  int region_1_page_table_size = UP_TO_PAGE(VMEM_1_SIZE) >> PAGESHIFT;
  pcb_t *child_pcb = allocate_pcb();
  if (child_pcb == NULL) {
    return ERROR;
  }

  int child_pid = helper_new_pid(child_pcb->region_1_page_table);
  child_pcb->pid = child_pid;
  child_pcb->parent = running_process;
  memcpy(child_pcb->uctxt, running_process->uctxt, sizeof(UserContext));
  child_pcb->rc = 0;
  running_process->rc = running_process->pid;

  // walk through the page table and copy over all allocated pages into a buffer page (with a new pfn)
  // Note: because our TLB caches pages, we need to either move our buffer page down or overwrite that single page
  // in our TLB. I choose the latter.
  int bufpage_index = (KERNEL_STACK_BASE >> PAGESHIFT) - 1;
  pte_t *bufpage = &region_0_page_table[bufpage_index];
  for (int i=0; i<region_1_page_table_size; i++) {
    if (running_process->region_1_page_table[i].valid) {
      int new_frame = get_free_frame(
          frame_table_global->frame_table, 
          frame_table_global->frame_table_size, 
          0
      );

      if (new_frame == MEMFULL) {
        TracePrintf(1, "FORK HANDLER: Ran out of free frames to allocate!\n");
        // clear the already-allocated frames on the page table
        delete_r1_page_table(child_pcb, i-1);
        // retire the new pcb
        helper_retire_pid(child_pcb->pid);
        free(child_pcb);
        return ERROR;
      }

      // use the page below the stack as a buffer to write stack pages into frames
      bufpage->valid = 1;
      bufpage->prot = (PROT_READ | PROT_WRITE);
      bufpage->pfn = new_frame;
      // keep the existing permissions, but update the pfn of the page
      child_pcb->region_1_page_table[i].valid = 1;
      child_pcb->region_1_page_table[i].prot = running_process->region_1_page_table[i].prot; 
      child_pcb->region_1_page_table[i].pfn = bufpage->pfn; 
      // write bytes in question to the frame 
      TracePrintf(5, "FORK HANDLER: Writing bytes %08x from %p to %p\n",
                  * (int *)(VMEM_1_BASE + (i << PAGESHIFT)),
                  (VMEM_1_BASE + (i << PAGESHIFT)), (VMEM_0_BASE + (bufpage_index << PAGESHIFT)));
      TracePrintf(5, "FORK HANDLER: Bufpage: Addr %x, valid %d, prot %d, pfn %d\n",
                  bufpage_index << PAGESHIFT, bufpage->valid, bufpage->prot, bufpage->pfn);
      memcpy((void *)(VMEM_0_BASE + (bufpage_index << PAGESHIFT)), (void *)(VMEM_1_BASE + (i << PAGESHIFT)), PAGESIZE);
      // flush the page from the TLB so it doesn't cache and overwrite the same frame
      bufpage->valid = 0;
      WriteRegister(REG_TLB_FLUSH, (int) (VMEM_0_BASE + (bufpage_index << PAGESHIFT)));
//      WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
    }
    else {
      child_pcb->region_1_page_table[i].valid = 0; 
    }
  }

  add_to_queue(ready_queue, child_pcb);
  // return the right thing for fork
  int rc = clone_process(child_pcb);

  TracePrintf(1, "Back from clone; return code is %d\n", running_process->rc);

  print_reg_1_page_table(running_process, 5, "POST FLUSH");
  print_reg_1_page_table_contents(running_process, 5, "POST FLUSH");
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  // if we've done the bookkeeping in our round robin/clock trap, then our running process should 
  // contain the correct pcb when returning from clone.
  return rc;
}

/*
 * Overwrite the address space of the old process with that of a new process
 * filename is the process to be executed
 * argvec is a null-terminated list of pointers to argument strings
 */
int handle_Exec(char *filename, char **argvec)
{
  TracePrintf(1, "EXEC_HANDLER: Attempting to load a new process with provided arguments\n");

  int rc = 0;

  // wipe out the page table for the old process 
  // load the ELF file from *filename
  // get the page table for the new process
  // place the arguments to be executed by the new process
  if ((rc = LoadProgram(filename, argvec, running_process)) != SUCCESS) {
    TracePrintf(1, "EXEC_HANDLER: Loading a process failed with exit code %d\n", rc);
    if (rc == -2) {
      TracePrintf(1, "Handle kill!\n", rc);
      delete_process(running_process, -2, true);
    }
    return rc;
  }

  TracePrintf(5, "Exec handler: my pid is %d\n", running_process->pid);
  print_reg_0_page_table(5, "POST EXEC");
  print_kernel_stack(5);
  print_reg_1_page_table(running_process, 5, "POST EXEC");
  print_reg_1_page_table_contents(running_process, 5, "POST EXEC");
  print_uctxt(running_process->uctxt, 5, "POST EXEC RUNNING PROCESS");

  // if load program returns an error, then exec should return error. Otherwise, return statement
  // will return to the original PC position and the return code doesn't matter. 
  return rc;
}

/*
 * Terminates the process, saving status for later retrieval by the parent
 * all other resources are freed
 * Orphaned processes will not store the status, free everything instead
 */
void handle_Exit(int status)
{
  TracePrintf(1, "EXIT: Handling exit with rc=%d for process with pid %d\n", status, running_process->pid);

  // if idle process exits, halt the machine
  if (running_process->pid == 1) {
    TracePrintf(0, "EXIT: Idle process is exiting; we're halting the kernel\n");
    Halt();
  }

  // iterate over children, setting their parent to be NULL
  pcb_t* next_child = running_process->children;

  while (next_child != NULL) {
    next_child->parent = NULL;
    next_child = next_child->next_sibling;
    TracePrintf(1, "EXIT: Looped\n");
  }
  running_process->children = NULL;

  TracePrintf(1, "EXIT: Calling the delete_process handler\n");
  delete_process(running_process, status, true);
}

/*
 * Gets the first exited child, if any, from the parent's collection of children
 */
pcb_t* get_first_exited_child(pcb_t* parent) {
  TracePrintf(1, "GET_FIRST_EXITED_CHILD: Checking children\n");
  pcb_t* next_child = parent->children;
  if (next_child == NULL) {
    TracePrintf(1, "GET_FIRST_EXITED_CHILD: ARRAY IS NULL!!!\n");
    return NULL;
  }

  while (next_child != NULL) {
    TracePrintf(1, "GET_FIRST_EXITED_CHILD: child %d has exited %d\n", next_child->pid, (int)(next_child->hasExited));
    if (next_child->hasExited) {
      // remove the child from the parent's collection
      if (next_child != NULL && parent->children == next_child) {
        parent->children = next_child->prev_sibling;
      }
      if (next_child != NULL && next_child->prev_sibling != NULL) {
        next_child->prev_sibling->next_sibling = next_child->next_sibling;
      }
      if (next_child != NULL && next_child->next_sibling != NULL) {
        next_child->next_sibling->prev_sibling = next_child->prev_sibling;
      }

      return next_child;
    }

    next_child = next_child->next_sibling;
  }

  return NULL;
}

/*
 * Collect PID and exit status of some child process
 * returns immediately if child is already dead
 * ERROR if calling process has no remaining children, alive or dead
 * otherwise waits until next child exits
 */
int handle_Wait(int *status_ptr)
{
  TracePrintf(1, "HANDLE_WAIT: triggered for process %d\n", running_process->pid);

  if (status_ptr != NULL && check_memory(status_ptr, sizeof (int), false, true, false, false) == ERROR) {
    TracePrintf(1, "HANDLE_WAIT: Provided a pointer to invalid memory\n");
    return ERROR;
  }

  // return ERROR immediately if no remaining children, alive or dead
  if (running_process->children == NULL) {
    TracePrintf(1, "HANDLE_WAIT: Error: no children remaining for %d\n", running_process->pid);
    if (status_ptr != NULL) {
      *status_ptr = ERROR;
    }
    return ERROR;
  }

  // check child processes on the PCB
  pcb_t* exited = get_first_exited_child(running_process);

  // return immediately if the child is already dead
  if (exited != NULL) {
    TracePrintf(1, "HANDLE_WAIT: Exited child found for parent %d with pid %d\n", running_process->pid, exited->pid);
    int status = exited->rc;
    if (status_ptr != NULL) {
      *status_ptr = status;
    }
    return status;
  }

  // otherwise:
  else {
    //    block parent until next child exits
    TracePrintf(1, "HANDLE_WAIT: blocking process %d and waiting for child death\n", running_process->pid);
    running_process->waitingForChildExit = true;
    install_next_from_queue(running_process, 1);
    // NOTE -- this runs when a child dies and signals its parent
    //    set the status_ptr and return
    TracePrintf(1, "HANDLE_WAIT: Back to process %d after child death\n", running_process->pid);
    exited = get_first_exited_child(running_process);
    // this should never be NULL
    if (exited == NULL) {
      TracePrintf(1, "HANDLE_WAIT: Critical error: exited is NULL after swapping back to parent %d\n", running_process->pid);
      Halt();
    }
    else {
      TracePrintf(1, "HANDLE_WAIT: Exited child found for parent %d with pid %d\n", running_process->pid, exited->pid);
      running_process->waitingForChildExit = false;
      int status = exited->rc;
      if (status_ptr != NULL) {
        *status_ptr = status;
      }
      switch_between_processes(running_process, exited);
      return status;
    }
  }
}

/*
 * Gets the PID of the calling process
 */
int handle_GetPid(void)
{
  // get the PID off the PCB
  return running_process->pid;
}

/*
 * Sets the brk
 * ERROR on failure, 0 on success
 */
int handle_Brk(void *addr)
{
  //error handling
  if (addr < (void *)VMEM_1_BASE) {
    TracePrintf(1, "Error: New brk address provided is in Region 0 (kernel memory).\n");
    return ERROR;
  }
  else if (addr > (void *)VMEM_1_LIMIT) {
    TracePrintf(1, "Error: New brk address provided is above writable memory.\n");
    return ERROR;
  } 
  
  pte_t *region_1_page_table = running_process->region_1_page_table;
  int addr_page = UP_TO_PAGE(addr - VMEM_0_LIMIT) >> PAGESHIFT;
  int current_brk_page = 0;
  int region_1_page_table_size = VMEM_1_SIZE >> PAGESHIFT;
  bool brkfound = false;

  TracePrintf(3, "Addr page is %d\n", addr_page);
  if (addr_page >= region_1_page_table_size) {
    TracePrintf(1, "Error: heap overflow. Unable to allocate memory past heap boundary\n");
    return ERROR;
  }

  TracePrintf(5, "=====Region 1 Page Table Before SetBrk (%d pages)=====\n", region_1_page_table_size);
  print_reg_1_page_table(running_process, 5, "");

  //find the brk
  while (!brkfound && current_brk_page < region_1_page_table_size){
    if (!region_1_page_table[current_brk_page].valid) {
      brkfound = true;
    }
    current_brk_page++;
  }
  current_brk_page--;
  TracePrintf(1, "SETBRK: Current brk found at %d pages\n", current_brk_page);

  // check to make sure we aren't going to grow into the user stack
  if (region_1_page_table[addr_page].valid) {
    TracePrintf(1, "SETBRK: Preventing growth into the user stack\n", current_brk_page);
    return ERROR;
  }

  int first_possible_free_frame = 0;
  if (addr_page > current_brk_page) {
    TracePrintf(3, "SETBRK: Will try to find memory to allocate more frames\n");
    // error out if we don't have enough memory
    if (addr_page-current_brk_page > get_num_free_frames(frame_table_global->frame_table,
                                                                  frame_table_global->frame_table_size)
        ) {
      TracePrintf(1, "SETBRK: SetBrk did not find enough memory for the whole malloc to succeed\n");
      return ERROR;
    }

    TracePrintf(3, "SETBRK: SetBrk found enough memory for malloc to succeed\n");
    while (addr_page > current_brk_page) {
      int new_frame_num = get_free_frame(frame_table_global->frame_table,
                                          frame_table_global->frame_table_size,
                                          first_possible_free_frame);
      if (new_frame_num == -1) {
        TracePrintf(1, "SETBRK: SetBrk was unable to allocate a new frame...\n");
        return ERROR;
      }
      region_1_page_table[current_brk_page].valid = 1;
      region_1_page_table[current_brk_page].prot = (PROT_READ | PROT_WRITE);
      region_1_page_table[current_brk_page].pfn = new_frame_num;
      current_brk_page++;
    }

    TracePrintf(5, "=====Region 1 Page Table After SetBrk=====\n");
    print_reg_1_page_table(running_process, 5, "");

    TracePrintf(1, "SETBRK: Brk set to %d pages\n", current_brk_page);

    return SUCCESS;
  }
  else {
    // TODO -- does this code path ever execute?
    TracePrintf(1, "SETBRK: SetKernelBrk found that we don't need to allocate more frames\n");
    if (addr_page < running_process->brk_floor) {
      TracePrintf(1, "SETBRK: Cannot set brk below the original size of the user heap\n");
      return ERROR;
    }

    while (addr_page < current_brk_page) {
      TracePrintf(1, "SETBRK: Deleting a page from the page table...\n");
      int discard_frame_number = region_1_page_table[current_brk_page].pfn;
      frame_table_global->frame_table[discard_frame_number] = 0;
      region_1_page_table[current_brk_page].valid = 0;
      current_brk_page--;
    }

    TracePrintf(1, "SETBRK: Brk set to %d pages\n", current_brk_page);

    return SUCCESS;
  }
}

/*
 * The calling process is blocked until at least clock ticks clock interrupts have occurred after the call. Upon
completion of the delay, the value 0 is returned.
If clock ticks is 0, return is immediate. If clock ticks is less than 0, time travel is not carried out, and
ERROR is returned instead.
 */
int handle_Delay(int clock_ticks)
{
  TracePrintf(1, "DELAY: Delaying for %d clock ticks\n", clock_ticks);
  // return ERROR if clock_ticks is negative
  if (clock_ticks < 0) {
    return ERROR;
  }

  // return 0 if clock_ticks is zero
  if (clock_ticks == 0) {
    return SUCCESS;
  }

  // make sure that we don't bring straggling processes along
  running_process->next_pcb = NULL;
  running_process->prev_pcb = NULL;

  // otherwise, block the process for clock_ticks (put in delay collection)
  running_process->delayed_clock_cycles = clock_ticks;

  // stick in at the head of the linked list
  running_process->next_pcb = delayed_processes;
  delayed_processes = running_process;

  if (delayed_processes->next_pcb != NULL) {
    // set the backpointer of the previous head
    delayed_processes->next_pcb->prev_pcb = delayed_processes;
  }

  pcb_t* old_process = running_process;

  // get the next process from the queue
  install_next_from_queue(old_process, 1);

  return SUCCESS;
}