#include <ykernel.h>
#include "../kernel_start.h"
#include "../data_structures/pcb.h"
#include "../data_structures/queue.h"
#include "../data_structures/frame_table.h"
#include "../debug_utils/debug.h"

extern frame_table_struct_t *frame_table_global;
extern pcb_t* running_process;
extern pcb_t* idle_process;                                           // the special idle process; use when nothing is in ready queue
extern bool is_idle;                                                  // if is_idle, we won't put the process back on the ready queue
extern queue_t* ready_queue;
extern void *trap_handler[16];
extern pte_t *region_0_page_table;
/*
 * Fork the process and create a new, separate address space
 * TODO: Copy on write, eventually
 */
int handle_Fork(void)
{
  TracePrintf(1, "PRE FORK\n\n");
  print_reg_0_page_table(1);  

  pcb_t *child_pcb = allocate_pcb();
  // by copying our current pcb, we get our PC, SP, and BP for free (in uctxt)
  child_pcb = memcpy(child_pcb, running_process, sizeof(pcb_t));
  // create the child's pid and region 1 page table by hand, since those are the 
  // unique structures not created by pcb initialization functions
  int child_pid = helper_new_pid(child_pcb->region_1_page_table);
  child_pcb->pid = child_pid;
  //TODO: copy all of region 1 page table into another, as was done with kernel stack in KCCopy

  int rc = clone_process(child_pcb);
  print_reg_1_page_table(child_pcb, 1);
  child_pcb->parent = running_process;
  add_to_queue(ready_queue, child_pcb);
  //TODO: deal with return codes
  TracePrintf(1, "POST FORK\n\n");
  print_reg_0_page_table(1); 
  return rc;
}

/*
 * Overwrite the address space of the old process with that of a new process
 * filename is the process to be executed
 * argvec is a null-terminated list of pointers to argument strings
 */
int handle_Exec(char *filename, char **argvec)
{
  // wipe out the page table for the old process
  // load the ELF file from *filename
  // get the page table for the new process
  // place the arguments to be executed by the new process
}

/*
 * Terminates the process, saving status for later retrieval by the parent
 * all other resources are freed
 * Orphaned processes will not store the status, free everything instead
 */
void handle_Exit(int status)
{
  // wipe out the page table for the process
  // free all other resources
  // check to see if the parent is dead; if so, completely delete the PCB
  // otherwise:
  //  save the status on the PCB
  //  place the parent on the ready queue if the parent is waiting for exit
}

/*
 * Collect PID and exit status of some child process
 * returns immediately if child is already dead
 * ERROR if calling process has no remaining children, alive or dead
 * otherwise waits until next child exits
 */
int handle_Wait(int *status_ptr)
{
  // check child processes on the PCB
  // return ERROR immediately if no remaining children, alive or dead
  // return immediately if the child is already dead
  // otherwise:
  //    block parent until next child exits
  //    set the status_ptr and return
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
  for (int i = 0; i < region_1_page_table_size; i++) {
    if (region_1_page_table[i].valid) {
      TracePrintf(5, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                  VMEM_1_BASE + (i << PAGESHIFT),
                  VMEM_1_BASE + ((i+1) << PAGESHIFT)-1,
                  region_1_page_table[i].valid,
                  region_1_page_table[i].pfn
      );
    }
  }

  //find the brk
  while (!brkfound && current_brk_page < region_1_page_table_size){
    if (!region_1_page_table[current_brk_page].valid) {
      brkfound = true;
    }
    current_brk_page++;
  }
  TracePrintf(1, "SETBRK: Current brk found at %d pages\n", current_brk_page);

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
    for (int i = 0; i < region_1_page_table_size; i++) {
      if (region_1_page_table[i].valid) {
        TracePrintf(5, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                    VMEM_1_BASE + (i << PAGESHIFT),
                    VMEM_1_BASE + ((i+1) << PAGESHIFT)-1,
                    region_1_page_table[i].valid,
                    region_1_page_table[i].pfn
        );
      }
    }

    TracePrintf(1, "SETBRK: Brk set to %d pages\n", current_brk_page);

    return SUCCESS;
  }
  else {
    // TODO -- does this code path ever execute?
    TracePrintf(1, "SETBRK: SetKernelBrk found that we don't need to allocate more frames\n");
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

  // otherwise, block the process for clock_ticks (put in delay queue)
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