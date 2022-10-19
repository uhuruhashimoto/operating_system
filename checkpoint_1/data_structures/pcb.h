#ifndef CURRENT_CHUNGUS_PCB
#define CURRENT_CHUNGUS_PCB

#include <ykernel.h>
#include "stdbool.h"

/*
* Our pcb stores the following types of info:
* 0. PID
* 1. Page table info and contexts
*     We need to store the user page table. Because the kernel page table
*     doesn't move, we simply store the new kernel stack.
* 2. Contexts
*     We store both user and kernel contexts. 
* 3. Proc Death Info
*   We store exit codes and a flag to check whether the process is dead.
* 4. Proc Parent/Child info
*   We store parent/child info in the PCB (to respond to process death
*   by updating the children.)
* 5. Queue info (embedded)
*/
typedef struct pcb {
  int pid;
  pte_t *kernel_stack;
  pte_t *region_1_page_table;
  UserContext uctxt;
  KernelContext kctxt;

  bool hasExited;                                      // whether the process is dead yet
  int rc;                                              // the return code of the process
  struct pcb *next_pcb;                                     // the next pcb in the queue
  struct pcb *prev_pcb;                                     // the previous pcb in the queue
  struct pcb *children;                                     // null unless there are children
  int num_children;                                    // 0 unless there are children
  struct pcb *parent;                                       // the parent, if any
} pcb_t;

#endif //CURRENT_CHUNGUS_PCB