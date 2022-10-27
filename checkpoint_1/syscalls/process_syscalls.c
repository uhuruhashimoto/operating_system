#include <ykernel.h>
#include "../kernel_start.h"

/*
 * Fork the process and create a new, separate address space
 */
int handle_Fork(void)
{
  // creates a new PID for the new process, coping over execution location
  // create new P1 page table
  // copy used pages of old process to the new process
  // finagle the return codes
  // sticks the new process in the ready queue
  // return
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
}

/*
 * Sets the brk
 * ERROR on failure, 0 on success
 */
int handle_Brk(void *addr)
{
  // Rounds the brk up to the next page
  // if increasing the size of the brk:
  //  create valid pages in the page table for all new memory below the brk
  // else:
  //  invalidates pages on the page table above the brk
  // sets the brk on the PCB
  // ERROR on failure, 0 on success
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

  // otherwise, block the process for clock_ticks (put in delay queue)
  running_process->delayed_clock_cycles = clock_ticks;
  if (delayed_processes == NULL) {
    delayed_processes = running_process;
  }
  else {
    pcb_t* next_proc = delayed_processes;
    while (next_proc->next_pcb != NULL) {
      // do nothing
    }
    // point them at one another, sticking this process in the queue
    next_proc->next_pcb = running_process;
    running_process->prev_pcb = next_proc;
  }

  pcb_t* old_process = running_process;

  // get the next process from the queue
  install_next_from_queue(old_process);

  return SUCCESS;
}