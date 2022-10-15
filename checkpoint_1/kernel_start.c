// Virtual Memory specifics
/*
* Our system uses two page tables, one in region 0 (kernel access) and one in region 1 (user access) to give 
* the kernel the illusion that each process has the full amount of VIRTUAL memory. 
* 
* Region 0 Page Table
* To initialize our Region 0 page tale, we create a page table in a region of memory such that the physical
* address and the virtual address are identical; that is, we map pages to frames of the same address. This page 
* table is of fixed size, and lives in the kernel heap.
* 
* Region 1 page table
* The region 1 page table is the place that individual process page tables are stored. Every process PCB will contain
* or point to the process page table, and these pointers will be written to the registers that tell the MMU where the 
* region 1 page table is when the process is run (and the TLB will be flushed). 
*
* We use the provided pmem size to keep track of a frame table - a bit vector that tracks which frames are mapped.
* (See section 2.2.6, p. 22 of Yalnix Manual)
*/

#include <ykernel.h>
#include "kernel_start.h"
#include "trap_handlers/trap_handlers.h"
#include "data_structures/pcb.h"
#include "data_structures/queue.h"

extern void *_kernel_data_start;
extern void *_kernel_data_end;
extern void *_kernel_orig_brk;

/*
 * Behavior:
 *  Set up virtual memory
 *  Set up trap handlers
 *  Instantiate an idlePCB
 */
void KernelStart(char *cmd args[], unsigned int pmem_size, UserContext *uctxt) {
  pte_t kernel_page;
  // We use macros to calculate page table size, and provided memory size to calculate frame table size
  pte_t *region_0_page_table = malloc(sizeof(pte_t) * UP_TO_PAGE(VMEM_0_SIZE)));
  pte_t *region_1_page_table = malloc(sizeof(pte_t) * UP_TO_PAGE(VMEM_1_SIZE));
  int *frame_table = malloc(sizeof(int) * UP_TO_PAGE(pmem_size) / PAGESIZE );
  
  /*
  To map the kernel memory into its region 0 page table, we break 
  its text and heap into PAGESIZE-length chunks, and put them
  into the page table with self-referential addresses.
  */
  int beginning_page = UP_TO_PAGE(PMEM_BASE) << PAGESHIFT;
  int num_kernel_pages = UP_TO_PAGE(_kernel_orig_brk) / PAGESIZE;
  for (int page_ind = 0; page_ind < num_kernel_pages; page_ind++) {
      kernel_page -> valid = 1;
      frame_table[beginning_page + page_index] = 1;
      kernel_page -> prot = (page_ind < kernel_text_end_page) ? (PROT_READ | PROT_EXE) : (PROT_READ | PROT_WRITE);
      kernel_page -> pfn = &region_0_page_table[page_ind];
  }

  //TODO: do we need to write the stack and globals too? I'm not sure; up to the brk might be fine.

  // update registers
  WriteRegister(REG_PTRBR0, &page_table_reg_0);
  WriteRegister(REG_PTLR0, num_kernel_pages);

  // turn on virtual memory permanently
  WriteRegister(REG_VM_ENABLE, 1);

  // TRAP HANDLERS
  // write base pointer of trap handlers to REG_VECTOR_BASE (how?)
  // set up trap handler array
  trap_handler[TRAP_VECTOR_SIZE];
  trap_handler[TRAP_KERNEL] = &handle_trap_kernel;
  trap_handler[TRAP_CLOCK] = &handle_trap_clock;
  trap_handler[TRAP_ILLEGAL] = &handle_trap_illegal;
  trap_handler[TRAP_MEMORY] = &handle_trap_memory;
  trap_handler[TRAP_MATH] = &handle_trap_math;
  trap_handler[TRAP_TTY_RECEIVE] = &handle_trap_tty_receive;
  trap_handler[TRAP_TTY_TRANSMIT] = &handle_trap_tty_transmit;
  trap_handler[TRAP_DISK] = &handle_trap_tty_unhandled;
  // handle all other slots in the trap vector
  for (int i=8; i<16; i++) {
    trap_handler[i] = &handle_trap_tty_unhandled;
  }

  // TODO -- allocate memory for the ready queue

  //TODO: create idle PCB and its page table, and write it to the correct registers (and store its PCB, and
  //page table, in the heap)
  // Then stick the idle PCB into the running_process global

}

/*
* Set the kernel brk based on whether or not virtual memory is enabled. This will be a syscall used by malloc and other
* higher-level user/library dynamic allocation calls.
* In case of any error, the value ERROR is returned.
*/
int SetKernelBrk(void *addr) {
  //if vmem is not enabled, set the brk to the specified address above _kernel_origin_brk
  //otherwise, set the brk assuming the address is virtual (a normal brk syscall)
  return 0
}