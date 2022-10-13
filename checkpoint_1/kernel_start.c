// Virtual Memory specifics
/*
* Our system uses two page tables, one in region 0 (kernel access) and one in region 1 (user access).
* To initialize our page tables, we create a page table in a region of memory such that the physical
* address and the virtual address are identical; that is, some offset from PMEM_BASE.
* 
* We store our page tables as globals (above the stack) and update the registers that tell the MMU 
* where to find them. We also keep track of a frame table - a bit vector that tracks which frames are mapped.
* (See section 2.2.6, p. 22 of Yalnix Manual)
*/

#include "kernel_start.h"
#include "trap_handlers/trap_handlers.h"
#include "data_structures/pcb.h"
#include "data_structures/queue.h"

extern void *_kernel_data_start;
extern void *_kernel_data_end;
extern void *_kernel_orig_brk;

pte_t *region_0_page_table;
pte_t *region_1_page_table;
unsigned char *frame_table; 
bool vmem_enabled = False;

/*
 * Behavior:
 *  Set up virtual memory
 *  Set up trap handlers
 *  Instantiate an idlePCB
 */
void KernelStart(char *cmd args[], unsigned int pmem_size, UserContext *uctxt) {
  // define memory geometry, adding an extra page if necessary to map entire region
  int page_table_size = UP_TO_PAGE(pmem_size) << PAGESHIFT;
  int beginning_page = UP_TO_PAGE(PMEM_BASE) << PAGESHIFT;
  int kernel_text_end_page = UP_TO_PAGE(_kernel_orig_brk) << PAGESHIFT;
  int first_user_page = DOWN_TO_PAGE(&region_1_page_table) << PAGESHIFT;
  int new_brk_page;
  pte_t kernel_page;
  pte_t user_page;

  //allocate structures and update brk
  region_0_page_table = malloc(sizeof(pte_t) * page_table_size);
  region_1_page_table  = malloc(sizeof(pte_t) * page_table_size);

  // set the brk, making the assumption that both page tables are allocated on the heap, which
  // grows upwards. Thus, the top would be at the end of the most recently allocated structure.
  if (SetKernelBrk(&region_1_page_table[page_table_size - 1]) == ERROR) {
    TracePrint("Error moving the brk to %p\n", &region_1_page_table[page_table_size - 1]);
    exit(ERROR);
  }

  //Define kernel text permissions as R-X, and heap as RW.
  new_brk_page = DOWN_TO_PAGE(&region_1_page_table[page_table_size - 1]) << PAGESHIFT;
  for (int page_ind = 0; page_ind < new_brk_page; page_ind++) {
      kernel_page -> valid = 1;
      frame_table[beginning_page + page_index] = 1;
      kernel_page -> prot = (page_ind < kernel_text_end_page) ? (PROT_READ | PROT_EXE) : (PROT_READ | PROT_WRITE);
      kernel_page -> pfn = &region_0_page_table[page_ind];
  }

  // write a single page to the page table
  user_page -> valid = 1;
  frame_table[first_user_page] = 1;
  user_page -> prot = (PROT_READ | PROT_WRITE);
  user_page -> pfn = &region_1_page_table[first_user_page];

  // update registers
  WriteRegister(REG_PTRBR0, &page_table_reg_0);
  WriteRegister(REG_PTLR0, num_kernel_pages);
  WriteRegister(REG_PTRBR1, &page_table_reg_1);
  WriteRegister(REG_PTLR1, 1);

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

  // TODO -- IDLE PCB
  // TODO -- stick the idle PCB into the running_process global

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