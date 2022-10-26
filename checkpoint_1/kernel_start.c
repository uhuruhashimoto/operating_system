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

// Before initializing an idle page, we need to set up our data structures (both our PCBs and 
// their running, ready, blocked, and defunct "queues"). These will be laid out as follows: 
// Running: global
// Ready: queue
// Blocked: multiple queues and pipes
// Defunct: some kind of linked list-like structure (analogous to the one we'll use to count ticks
// on PCBs for our clock Delay syscall).

#include <ykernel.h>
#include "kernel_start.h"
#include "trap_handlers/trap_handlers.h"
#include "data_structures/pcb.h"
#include "data_structures/queue.h"

extern void *_kernel_data_start;
extern void *_kernel_data_end;
extern void *_kernel_orig_brk;
int vmem_on = 0;

/*
* We are given addresses in bytes corresponding to the following kernel address space:
* ---------- Top of region 0 (KERNEL_STACK_LIMIT or VMEM_0_SIZE)
* Stack
* ---------- Stack base (KERNEL_STACK_BASE)
* empty
* ---------- End of heap (_kernel_data_end)
* Heap
* Data
* -------- Start of data (_kernel_data_start)
* Text
* -------- Base (PMEM_BASE)
* 
* For each of these addresses (lines), we bit-shift the address by PAGESHIFT to find out 
* the page number it will correspond to in our page table. 
*/
void KernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *uctxt) {
  // full sizes
  int total_pmem_pages = UP_TO_PAGE(pmem_size) >> PAGESHIFT;
  int region_0_page_table_size = UP_TO_PAGE(VMEM_0_SIZE) >> PAGESHIFT;

  // Address sizes
  int kernel_text_start_page = UP_TO_PAGE(PMEM_BASE) >> PAGESHIFT;
  int kernel_data_start_page = UP_TO_PAGE(_kernel_data_start - PMEM_BASE) >> PAGESHIFT;
  int kernel_data_end_page = UP_TO_PAGE(_kernel_data_end - PMEM_BASE) >> PAGESHIFT;
  int stack_start_page = UP_TO_PAGE(KERNEL_STACK_BASE - PMEM_BASE) >> PAGESHIFT;
  int stack_end_page = UP_TO_PAGE(KERNEL_STACK_LIMIT - PMEM_BASE) >> PAGESHIFT;

  // Page table setup
  region_0_page_table = malloc(sizeof(pte_t) * region_0_page_table_size);
  current_kernel_brk_page = malloc(sizeof(int));
  *current_kernel_brk_page = kernel_data_end_page;

  // Frame table setup
  // Create frame tracking bit vector and put it in the global along with its size
  frame_table_struct = malloc(sizeof(frame_table_struct));
  char *frame_table = malloc(sizeof(char) * total_pmem_pages);
  frame_table_struct->frame_table = frame_table;
  frame_table_struct->frame_table_size = pmem_size;

  // helpers to walk through page table
  pte_t kernel_page;
  int *addr;

  // Address checks
  TracePrintf(1, "Physical memory size is %x bytes, %d pages\n", pmem_size, total_pmem_pages);
  TracePrintf(1, "Region 0 size is %x bytes, %d pages\n", VMEM_0_SIZE, region_0_page_table_size); 
  TracePrintf(1, "-------------------------\n");
  TracePrintf(1, "Kernel text start page is addr %x, page %d \n", PMEM_BASE, kernel_text_start_page);
  TracePrintf(1, "Kernel data start page is addr %x, page %d \n", _kernel_data_start - PMEM_BASE, kernel_data_start_page);
  TracePrintf(1, "Kernel data end page is addr %x, page %d \n", _kernel_data_end - PMEM_BASE, kernel_data_end_page);
  TracePrintf(1, "Kernel stack start page is addr %x, page %d \n", KERNEL_STACK_BASE - PMEM_BASE, stack_start_page);
  TracePrintf(1, "Kernel stack end page is addr %x, page %d \n", KERNEL_STACK_LIMIT - PMEM_BASE, stack_end_page);

  int kernel_pageind = 0;
  int max_consumed_frame = 0;
  for (kernel_pageind = 0; kernel_pageind < total_pmem_pages; kernel_pageind++) {
    addr = (int *) (PMEM_BASE + PAGESIZE * kernel_pageind);
    // Kernel text is implied to exist from the physical base (NULL) until kernel data begins
    if (kernel_pageind < (kernel_data_start_page-1)) {
      if (kernel_pageind == 0) {
        kernel_page.valid = 0;
      }
      else {
        kernel_page.valid = 1;
      }
      frame_table[kernel_pageind] = 1;
      kernel_page.prot = (PROT_READ | PROT_EXEC);
      kernel_page.pfn = kernel_pageind;
      region_0_page_table[kernel_pageind] = kernel_page;
      max_consumed_frame++;
    }
    // After text, we give data RW permissions
    else if (kernel_pageind >= (kernel_data_start_page-1) && kernel_pageind < kernel_data_end_page) {
      frame_table[kernel_pageind] = 1;
      kernel_page.valid = 1;
      kernel_page.prot = (PROT_READ | PROT_WRITE);
      kernel_page.pfn = kernel_pageind;
      region_0_page_table[kernel_pageind] = kernel_page;
      max_consumed_frame++;
    }
    // Until the stack, we add invalid pages to give the kernel the illusion of the whole reg. 0 memory space
    else if (kernel_pageind >= kernel_data_end_page && kernel_pageind < stack_start_page) {
      frame_table[kernel_pageind] = 1;
      kernel_page.valid = 0;
      kernel_page.pfn = kernel_pageind;
      region_0_page_table[kernel_pageind] = kernel_page;
      max_consumed_frame++;
    }
    // then we mark the stack as valid 
    else if (kernel_pageind >= stack_start_page && kernel_pageind < stack_end_page) {
      frame_table[kernel_pageind] = 1;
      kernel_page.valid = 1;
       kernel_page.prot = (PROT_READ | PROT_WRITE);
      kernel_page.pfn = kernel_pageind;
      region_0_page_table[kernel_pageind] = kernel_page;
      max_consumed_frame++;
    }
    // then we map the remainder of free physical frames
    else {
      frame_table[kernel_pageind] = 0;
    }

  }

  // update registers
  WriteRegister(REG_PTBR0, (int) region_0_page_table);
  WriteRegister(REG_PTLR0, region_0_page_table_size);

  // turn on virtual memory permanently
  WriteRegister(REG_VM_ENABLE, 1);
  vmem_on = 1;


  // TRAP HANDLERS
  // set up trap handler array
  trap_handler[TRAP_KERNEL] = &handle_trap_kernel;
  trap_handler[TRAP_CLOCK] = &handle_trap_clock;
  trap_handler[TRAP_ILLEGAL] = &handle_trap_illegal;
  trap_handler[TRAP_MEMORY] = &handle_trap_memory;
  trap_handler[TRAP_MATH] = &handle_trap_math;
  trap_handler[TRAP_TTY_RECEIVE] = &handle_trap_tty_receive;
  trap_handler[TRAP_TTY_TRANSMIT] = &handle_trap_tty_transmit;
  trap_handler[TRAP_DISK] = &handle_trap_unhandled;
  // handle all other slots in the trap vector
  for (int i=8; i<16; i++) {
    trap_handler[i] = &handle_trap_unhandled;
  }
  // write base pointer of trap handlers to REG_VECTOR_BASE
  WriteRegister(REG_VECTOR_BASE, (int) trap_handler);


  // Create an idle pcb, with PC pointing to the kernel idle function
  // copy over kernel stack so we can take it with us
  int num_kernel_stack_pages = stack_end_page - stack_start_page;
  pte_t *kernel_stack = malloc(sizeof(pte_t) *num_kernel_stack_pages);
  for (int i=0; i<num_kernel_stack_pages; i++) {
    kernel_stack[i] = region_0_page_table[i];
  }

  // set up region 1 page table
  int idle_stack_size = 2;
  pte_t idle_page;
  int page_table_reg_1_size = UP_TO_PAGE(VMEM_1_SIZE) >> PAGESHIFT;
  pte_t *region_1_page_table = malloc(sizeof(pte_t) * page_table_reg_1_size);
  for (int ind=0; ind<page_table_reg_1_size; ind++) {
    // set everything under the stack as non-valid (since the text is in the kernel and 
    // our loop shouldn't use any memory)
    idle_page.valid = 0;

    if (ind < page_table_reg_1_size - idle_stack_size) {
      region_1_page_table[ind] = idle_page;
    }
    // Here's the stack
    else {
      idle_page.valid = 1;
      idle_page.prot = (PROT_READ | PROT_WRITE);
      idle_page.pfn = max_consumed_frame;
      frame_table[max_consumed_frame] = 1;
      region_1_page_table[ind] = idle_page;
      max_consumed_frame++;
    }
  }

  KernelContext kctxt;
  uctxt -> pc = &DoIdle;
  uctxt -> sp = (void *) VMEM_1_LIMIT - 8; // 8...it's a magic number 
  int pid = helper_new_pid(region_1_page_table);
  pcb_t *idle_pcb = create_pcb(pid, kernel_stack, region_1_page_table, uctxt, &kctxt);
  running_process = idle_pcb;

  // update registers
  WriteRegister(REG_PTBR1, (int) region_1_page_table);
  WriteRegister(REG_PTLR1, page_table_reg_1_size);


  // when we return to userland, got to the idle process
  TracePrintf(1, "Leaving KernelStart...\n");
  return; 
}

/*
* Set the kernel brk based on whether or not virtual memory is enabled. This will be a syscall used by malloc and other
* higher-level user/library dynamic allocation calls. In case of any error, the value ERROR is returned.
*/
int SetKernelBrk(void *addr) {
  // error out if we don't have enough memory 
  if (!get_num_free_frames(frame_table_struct->frame_table, frame_table_struct->frame_table_size)) {
    return ERROR;
  }
  // error out if we're provided an invalid address
  if (addr < _kernel_data_start || addr > _kernel_data_end) {
    return ERROR;
  }
  // error out if we are trying to map the page right below the stack or the stack itself
  int stack_page_num = KERNEL_STACK_BASE >> PAGESHIFT;
  int addr_page = UP_TO_PAGE(addr - PMEM_BASE) >> PAGESHIFT;
  if (addr_page >= stack_page_num - 1) {
    return ERROR;
  }
  // if vmem is not enabled, set the brk to the specified address above _kernel_origin_brk (hit by kernel malloc)
  if (!vmem_on) {
    if (addr_page > *current_kernel_brk_page) {
      while (addr_page > *current_kernel_brk_page) {
        int new_frame_num = get_free_frame(frame_table_struct->frame_table, frame_table_struct->frame_table_size);
        region_0_page_table[*current_kernel_brk_page].valid = 1;
        region_0_page_table[*current_kernel_brk_page].prot = (PROT_READ | PROT_WRITE);
        region_0_page_table[*current_kernel_brk_page].pfn = new_frame_num;
        *current_kernel_brk_page++;
      }
      return 0;
    }
    else {
      while (addr_page < *current_kernel_brk_page) {
        int discard_frame_number = region_0_page_table[*current_kernel_brk_page].pfn;
        frame_table_struct->frame_table[discard_frame_number] = 0;
        region_0_page_table[*current_kernel_brk_page].valid = 0;
        *current_kernel_brk_page--;
      }
      return 0;
    }
  }
  //otherwise, set the brk assuming the address is virtual (a normal brk syscall)
  else {
    // pte_t *region_1_brk_page_table = running_process->region_1_page_table;
    // int new_frame_num = get_free_frame(frame_table_struct->frame_table, frame_table_struct->frame_table_size);
    // pte_t brk_page;
    // brk_page.valid = 1;
    // brk_page.prot = (PROT_READ | PROT_WRITE);
    // brk_page.pfn = new_frame_num;
    // region_1_brk_page_table[free_page] = brk_page;
    return 0;
  }
}

/*
Idle PCB code (part of kernel text)
*/ 
void DoIdle(void) {
  while(1) {
    TracePrintf(1,"DoIdle\n");
    Pause();
  } 
}

//============================ FRAME TABLE HELPERS ==============================//
/*
return the number of free frames, to help during dynamic allocation. 
This assumes that the kernel is uninterruptable and needs no synchronization (mutexes, etc.)
*/
int get_num_free_frames(char *frame_table, int frame_table_size) {
  int numfree = 0;
  for (int i=0; i<frame_table_size; i++) {
    // sum the number of unused frame table frames (1 is used, 0 unused)
    numfree += (frame_table[i] ? 0 : 1);
  }
  return numfree;
}

/*
Traverses frame table bit vector to find the index
of the next free frame. This assumes that the kernel is uninterruptable
and needs no synchronization (mutexes, etc.)
*/
int get_free_frame(char *frame_table, int frame_table_size) {
  int i = 0;
  while (frame_table[i]) {
    if (i < frame_table_size) {
      i++;
    } 
    else {
      return MEMFULL;
    }
  }
  frame_table[i] = 1;
  return i;
}
