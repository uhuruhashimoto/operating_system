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
// #include "trap_handlers/trap_handlers.h"
#include "data_structures/pcb.h"
#include "data_structures/queue.h"

extern void *_kernel_data_start;
extern void *_kernel_data_end;
extern void *_kernel_orig_brk;
int vmem_on = 0;
int *current_kernel_brk; 

/*
 * Behavior:
 *  Set up virtual memory
 *  Set up trap handlers
 *  Instantiate an idlePCB
 */
void KernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *uctxt) {
  TracePrintf(1, "Kernel data start is %p\n", _kernel_data_start);
  TracePrintf(1, "Kernel orig brk is %p\n", _kernel_orig_brk);
  TracePrintf(1, "Kernel data end is %p\n", _kernel_data_end);

  TracePrintf(1, "PMEM Base: %p\n", PMEM_BASE);
  TracePrintf(1, "Pmem size: %x\n", pmem_size);
  TracePrintf(1, "BITS: %d\n", PAGESHIFT);
  TracePrintf(1, "Size of page table in bytes: %d\n", (pmem_size >> PAGESHIFT) * 4);
  TracePrintf(1, "Bytes needed bit field: %d\n", (pmem_size >> PAGESHIFT) / 8);
  TracePrintf(1, "Size of Region 0 is %d\n", VMEM_0_SIZE);

  int frame_table_size = UP_TO_PAGE(pmem_size) >> PAGESHIFT;
  int page_table_reg_0_size = UP_TO_PAGE(VMEM_0_SIZE) >> PAGESHIFT;
  int *frame_table = malloc(sizeof(int) * frame_table_size);
  pte_t *region_0_page_table = malloc(sizeof(pte_t) * page_table_reg_0_size);
  pte_t kernel_page;
  int *addr;

  int num_ktext_pages = UP_TO_PAGE(_kernel_data_start - PMEM_BASE) / PAGESIZE - 1;
  int num_kdata_pages = UP_TO_PAGE(_kernel_data_end - PMEM_BASE) / PAGESIZE;
  int num_kstack_start_page = UP_TO_PAGE(KERNEL_STACK_BASE - PMEM_BASE) / PAGESIZE;
  int num_total_kernel_pages = UP_TO_PAGE(VMEM_0_SIZE) / PAGESIZE;
  TracePrintf(1, "Total number of kernel pages: %d\n", num_total_kernel_pages);
  for (int pageind = 0; pageind < frame_table_size; pageind++) {
    addr = (int *) (PMEM_BASE + PAGESIZE * pageind);
    // Kernel text is implied to exist from the physical base (NULL) until kernel data begins
    if (pageind < num_ktext_pages) {
      frame_table[pageind] = 1;
      kernel_page.valid = 1;
      kernel_page.prot = (PROT_READ | PROT_EXEC);
      kernel_page.pfn = pageind; 
      region_0_page_table[pageind] = kernel_page;
    }
    // After text, we give data RW permissions
    else if (pageind >= num_ktext_pages && pageind < num_kdata_pages) {
     frame_table[pageind] = 1;
      kernel_page.valid = 1;
      kernel_page.prot = (PROT_READ | PROT_WRITE);
      kernel_page.pfn = pageind; 
      region_0_page_table[pageind] = kernel_page; 
    }
    // Until the stack, we add invalid pages to give the kernel the illusion of the whole reg. 0 memory space
    else if (pageind >= num_kdata_pages && pageind < num_kstack_start_page) {
      frame_table[pageind] = 1;
      kernel_page.valid = 0;
      kernel_page.pfn = pageind; 
      region_0_page_table[pageind] = kernel_page;  
    }
    // then we mark the stack as valid 
    else if (pageind >= num_kstack_start_page && pageind < num_total_kernel_pages) {
      frame_table[pageind] = 1;
      kernel_page.valid = 1;
       kernel_page.prot = (PROT_READ | PROT_WRITE);
      kernel_page.pfn = pageind; 
      region_0_page_table[pageind] = kernel_page; 
    }
    // then we map the remainder of free physical frames
    else {
      frame_table[pageind] = 0;
    }
  }

  // update registers
  WriteRegister(REG_PTBR0, (int) region_0_page_table);
  WriteRegister(REG_PTLR0, num_total_kernel_pages);

  // turn on virtual memory permanently
  WriteRegister(REG_VM_ENABLE, 1);
  vmem_on = 1;

  // // TRAP HANDLERS
  // // write base pointer of trap handlers to REG_VECTOR_BASE (how?)
  // // set up trap handler array
  // // trap_handler[TRAP_VECTOR_SIZE];
  // // trap_handler[TRAP_KERNEL] = &handle_trap_kernel;
  // // trap_handler[TRAP_CLOCK] = &handle_trap_clock;
  // // trap_handler[TRAP_ILLEGAL] = &handle_trap_illegal;
  // // trap_handler[TRAP_MEMORY] = &handle_trap_memory;
  // // trap_handler[TRAP_MATH] = &handle_trap_math;
  // // trap_handler[TRAP_TTY_RECEIVE] = &handle_trap_tty_receive;
  // // trap_handler[TRAP_TTY_TRANSMIT] = &handle_trap_tty_transmit;
  // // trap_handler[TRAP_DISK] = &handle_trap_tty_unhandled;
  // // // handle all other slots in the trap vector
  // // for (int i=8; i<16; i++) {
  // //   trap_handler[i] = &handle_trap_tty_unhandled;
  // // }

  // // TODO: Before initializing an idle page, we need to set up our data structures (both our PCBs and 
  // // their running, ready, blocked, and defunct "queues"). These will be laid out as follows: 
  // // Running: global
  // // Ready: queue
  // // Blocked: multiple queues and pipes
  // // Defunct: some kind of linked list-like structure (analogous to the one we'll use to count ticks
  // // on PCBs for our clock Delay syscall).

  // Create an idle pcb, with PC pointing to the kernel idle function
  int num_kernel_stack_pages = num_total_kernel_pages - num_kstack_start_page;
  pte_t *kernel_stack = malloc(sizeof(pte_t) *num_kernel_stack_pages);
  for (int i=0; i<num_kernel_stack_pages; i++) {
    kernel_stack[i] = region_0_page_table[i];
  }
  int page_table_reg_1_size = UP_TO_PAGE(VMEM_1_SIZE) >> PAGESHIFT;
  pte_t *region_1_page_table = malloc(sizeof(pte_t) * page_table_reg_1_size);
  KernelContext kctxt;
  uctxt -> pc = &DoIdle;
  uctxt -> sp = &region_1_page_table[page_table_reg_1_size -1];
  uctxt -> ebp =&region_1_page_table[page_table_reg_1_size -1]; 
  int pid = helper_new_pid(region_1_page_table);
  pcb_t *idle_pcb = create_pcb(pid, kernel_stack, region_1_page_table, uctxt, &kctxt);
  running_process = idle_pcb;

  // update registers
  WriteRegister(REG_PTBR1, (int) region_1_page_table);
  WriteRegister(REG_PTLR1, page_table_reg_1_size);
  // when we return to userland, got to the idle process
  return;
}

/*
* Set the kernel brk based on whether or not virtual memory is enabled. This will be a syscall used by malloc and other
* higher-level user/library dynamic allocation calls.
* In case of any error, the value ERROR is returned.
*/
int SetKernelBrk(void *addr) {
 TracePrintf(1, "At beginning of SetKernelBrk, addr is %p\n", addr); 
 return 0;
  // // error out if we don't have enough memory or if our address is invalid
  // if (addr < _kernel_data_start || addr > _kernel_data_end) {
  //   return ERROR;
  // }
  // //TODO: check if we have enough memory left to allocate
  // // if vmem is not enabled, set the brk to the specified address above _kernel_origin_brk (hit by kernel malloc)
  // if (!vmem_on) {
  //   current_kernel_brk = UP_TO_PAGE(_kernel_orig_brk + PAGESIZE);
  //   TracePrintf(1, "In physical memory, increasing brk to %p\n", current_kernel_brk);
  //   return 0;
  // }
  // //otherwise, set the brk assuming the address is virtual (a normal brk syscall)
  // else {
  //   TracePrintf(1, "In virtual memory, incrementing brk to %p\n", addr);
  //   current_kernel_brk = UP_TO_PAGE(addr);
  //   return 0;
  // }
}

// Idle PCB code (part of kernel text)
void DoIdle(void) {
  while(1) {
    TracePrintf(1,"DoIdle\n");
    Pause();
  } 
}
