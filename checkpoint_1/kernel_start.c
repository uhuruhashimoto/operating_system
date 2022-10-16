// Virtual Memory specifics
// (See section 2.2.6, p. 22 of Yalnix Manual)
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
*/

#include <ykernel.h>
#include "kernel_start.h"
#include "trap_handlers/trap_handlers.h"
#include "data_structures/pcb.h"
#include "data_structures/queue.h"
#define ERROR -1

extern void *_kernel_data_start;
extern void *_kernel_data_end;
extern void *_kernel_orig_brk;
int vmem_on = 0;

/*
 * Behavior:
 *  Set up virtual memory
 *  Set up trap handlers
 *  Instantiate an idlePCB
 */
void KernelStart(char *cmd args[], unsigned int pmem_size, UserContext *uctxt) {
  // We use macros to calculate page table size, and provided memory size to calculate frame table size
  int frame_table_size = UP_TO_PAGE(pmem_size) / PAGESIZE;
  pte_t *region_0_page_table = malloc(sizeof(pte_t) * UP_TO_PAGE(VMEM_0_SIZE)));
  pte_t *region_1_page_table = malloc(sizeof(pte_t) * UP_TO_PAGE(VMEM_1_SIZE));
  int *frame_table = malloc(sizeof(int) * frame_table_size);
  
  /*
  To map the kernel memory into its region 0 page table, we break 
  its text and heap into PAGESIZE-length chunks, and put them
  into the page table with self-referential addresses.

  Their permissions are as follows, towards foxes (bottom up):
  kernel text: valid, RX, frame 1
  kernel heap: valid, RW, frame 1
  -- empty space -- , frame 0
  kernel stack: valid, RW,  frame 1
  kernel globals: valid, RW, frame 1
  --- more empty space -- , frame 0

  We find the new brk page by dereferencing the last entry of our frame table 
  (the last data structure on the heap).
  We find the kernel stack address by creating a local and dereferencing it.
  */

  int page_ind;
  pte_t kernel_page;
  int beginning_page = UP_TO_PAGE(PMEM_BASE) << PAGESHIFT;
  int kernel_text_end_page = UP_TO_PAGE(_kernel_orig_brk) << PAGESHIFT; 
  int kernel_brk_end_page = UP_TO_PAGE(&frame_table[frame_table_size-1]) << PAGESHIFT;
  int last_kernel_page = UP_TO_PAGE(_kernel_data_end) << PAGESHIFT;
  int *stack_addr;
  *stack_addr = UP_TO_PAGE(&stack_addr) << PAGESHIFT;
  for (page_ind = beginning_page; page_ind < num_kernel_pages; page_ind++) {
      // Kernel text and anything under it
      if (page_ind <= kernel_text_end_page) {
        frame_table[beginning_page + page_index] = 1;
        kernel_page -> valid = 1;
        kernel_page -> prot = (PROT_READ | PROT_EXE);
        kernel_page -> pfn = &region_0_page_table[page_ind];
        region_0_page_table[page_ind] = kernel_page;
      }
      // Kernel heap
      else if (page_ind > kernel_text_end_page && page_ind <= kernel_brk_end_page) {
        frame_table[beginning_page + page_index] = 1;
        kernel_page -> valid = 1;
        kernel_page -> prot = (PROT_READ | PROT_WRITE);
        kernel_page -> pfn = &region_0_page_table[page_ind];
        region_0_page_table[page_ind] = kernel_page;

      } 
      // Empty space (still kernel memory)
      else if (page_ind > kernel_brk_end_page && page_ind <= *stack_addr) {
        frame_table[beginning_page + page_index] = 0;
      } 
      // Kernel stack and globals
      else if (page_ind > *stack_addr && page_ind <= last_kernel_page) {
        frame_table[beginning_page + page_index] = 1;
        kernel_page -> valid = 1;
        kernel_page -> prot = (PROT_READ | PROT_WRITE);
        kernel_page -> pfn = &region_0_page_table[page_ind];
        region_0_page_table[page_ind] = kernel_page;

      } 
      // Empty space
      else if (page_ind > last_kernel_page && page_ind <= frame_table_size) {
        frame_table[beginning_page + page_index] = 0;
      } 
  }

  // update registers
  WriteRegister(REG_PTRBR0, &page_table_reg_0);
  WriteRegister(REG_PTLR0, num_kernel_pages);

  // turn on virtual memory permanently
  WriteRegister(REG_VM_ENABLE, 1);
  vmem_on = 1;

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

  // TODO: Before initializing an idle page, we need to set up our data structures (both our PCBs and 
  // their running, ready, blocked, and defunct "queues"). These will be laid out as follows: 
  // Running: global
  // Ready: queue
  // Blocked: multiple queues and pipes
  // Defunct: some kind of linked list-like structure (analogous to the one we'll use to count ticks
  // on PCBs for our clock Delay syscall).

}

/*
* Set the kernel brk based on whether or not virtual memory is enabled. This will be a syscall used by malloc and other
* higher-level user/library dynamic allocation calls.
* In case of any error, the value ERROR is returned.
*/
int SetKernelBrk(void *addr) {
  //if vmem is not enabled, set the brk to the specified address above _kernel_origin_brk (hit by kernel malloc)
  if (!vmem_on) {
    if (addr > _kernel_data_end) {
      return ERROR;
    }
    intptr_t increment = addr - _kernel_orig_brk;
    //sbrk returns (void *) 0 if successful, and -1 if unsuccessful
    return *sbrk(increment);
  }
  //otherwise, set the brk assuming the address is virtual (a normal brk syscall)
  else {
    return brk(addr);
  }
}