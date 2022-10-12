// Virtual Memory specifics
/*
* Our system uses two page tables, one in region 0 (kernel access) and one in region 1 (user access).
* To initialize our page tables, we create a page table in a region of memory such that the physical
* address and the virtual address are identical; that is, some offset from PMEM_BASE.
* 
* We store our page tables as globals (above the stack) and update the registers that tell the MMU 
* where to find it. Under it, we keep track of a frame table - a bit vector that tracks which frames are mapped.
* (See section 2.2.6, p. 22 of Yalnix Manual)
*/

// another option: mmap pages to put the region 1 page table wherever we want in physical memory?

#include "kernel_start.h"
#include "trap_handlers/trap_handlers.h"
#include "data_structures/pcb.h"
#include "data_structures/queue.h"


bool vmem_enabled = False;
pte_t **page_table_reg_1[VMEM_1_SIZE / PAGESIZE];
pte_t **page_table_reg_0[VMEM_0_SIZE / PAGESIZE];
unsigned char **frame_table; 

/*
 * Behavior:
 *  Set up virtual memory
 *  Set up trap handlers
 *  Instantiate an idlePCB
 */
void KernelStart(char *cmd args[], unsigned int pmem_size, UserContext *uctxt) {

  // assuming page size and frame size are the same, set up bit vector using malloc,
  // and move the brk accordingly
  int num_total_frames = pmem_size / PAGESIZE;
  frame_table = malloc(sizeof(unsigned char) * num_total_frames);
  SetKernelBrk(&frame_table[num_total_frames - 1]);

  // create kernel page table: store all kernel data/stack/heap/text such
  // that its virtual address points to its identical physical address
  int which_kernel_page;
  int num_kernel_pages = (_kernel_data_end - _kernel_data_start) / PAGESIZE;
  while (which_kernel_page < num_kernel_pages) {
    pte_t kernel_page;
    kernel_page -> valid = 1;
    kernel_page -> prot = 101;
    kernel_page -> pfn = &page_table_reg_0[which_kernel_page];
    page_table_reg_0[which_kernel_page] = kernel_page;
    which_kernel_page += PAGESIZE;
  }

  // create user page table with one page
  pte_t user_page;
  user_page -> valid = 1;
  user_page -> prot = 101;
  user_page -> pfn = &page_table_reg_0[0];
  page_table_reg_0[0] = user_page;

  // update registers
  WriteRegister(REG_PTRBR0, &page_table_reg_0);
  WriteRegister(REG_PTLR0, num_kernel_pages);
  WriteRegister(REG_PTRBR1, &page_table_reg_1);
  WriteRegister(REG_PTLR1, 1);

  // turn on virtual memory permanently
  WriteRegister(REG_VM_ENABLE, 1);

  // TODO -- TRAP HANDLERS
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

int SetKernelBrk(void *addr) {
  //if vmem is not enabled, set the brk to the specified address above _kernel_origin_brk
  //otherwise, set the brk assuming the address is virtual (a normal brk syscall)
}