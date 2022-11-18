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
#include "kernel_utils.h"
#include "trap_handlers/trap_handlers.h"
#include "data_structures/pcb.h"
#include "data_structures/queue.h"
#include "data_structures/frame_table.h"
#include "data_structures/pipe.h"
#include "data_structures/lock.h"
#include "data_structures/tty.h"
#include "process_management/load_program.h"
#include "syscalls/io_syscalls.h"
#include "debug_utils/debug.h"


extern void *_kernel_data_start;
extern void *_kernel_data_end;
extern void *_kernel_orig_brk;
int vmem_on = 0;

//=================== KERNEL GLOBALS ===================//
/*
* Globals that persist indefinitely for the whole kernel:
    1. Memory storage, including the frame table and brk
    2. Process tracking, including pcbs and ready/idle/blocked queues
    3. Pipes, TTYs, Locks, etc
*/

// TRAP HANDLER
void *trap_handler[NUM_TRAP_FUNCTIONS];

// MEMORY
int current_kernel_brk_page;
frame_table_struct_t *frame_table_global;
pte_t *region_0_page_table;
char** cmd_args_global;

// PROCESSES
pcb_t* running_process;
pcb_t* idle_process;                                           // the special idle process; use when nothing is in ready queue
bool is_idle = false;                                          // if is_idle, we won't put the process back on the ready queue
queue_t* ready_queue;
pcb_t *delayed_processes = NULL;                               // a linked list of processes being delayed

// PIPES
pipe_t* pipes = NULL;
unsigned int min_possible_pipe_id = 0;
unsigned int max_pipe_id = -1;
unsigned int max_possible_pipe_id = 1000000;

// LOCKS
lock_t* locks = NULL;
unsigned int min_possible_lock_id = 2000000;
unsigned int max_lock_id = 1999999;
unsigned int max_possible_lock_id = 3000000;

// CVARS
cvar_t* cvars = NULL;
unsigned int min_possible_cvar_id = 4000000;
unsigned int max_cvar_id = 3999999;
unsigned int max_possible_cvar_id = 5000000;

//TERMINALS
tty_object_t *tty_objects[NUM_TERMINALS];
char tty_buffer[TTY_BUFFER_SIZE];

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
  cmd_args_global = cmd_args;

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
  if (region_0_page_table == NULL) {
    // Halt; kernel will be unable to boot
    TracePrintf(1, "KernelStart: Unable to allocate memory for region 0 page table. Halting.\n");
    Halt();
  }
  // kernelbrk page is the next UNMAPPED page -- i.e., one plus the kernel data end page
  current_kernel_brk_page = kernel_data_end_page+1;
  TracePrintf(1, "KernelBrk page initially set to %d\n", current_kernel_brk_page);

  // Frame table setup
  // Create frame tracking bit vector and put it in the global along with its size
  frame_table_global = malloc(sizeof(frame_table_global));
  if (frame_table_global == NULL) {
    TracePrintf(1, "KernelStart: Unable to allocate memory for frame table global. Halting.\n");
    Halt();
  }
  char *frame_table = malloc(sizeof(char) * total_pmem_pages);
  if (frame_table == NULL) {
    TracePrintf(1, "KernelStart: Unable to allocate memory for frame table. Halting.\n");
    Halt();
  }
  frame_table_global->frame_table = frame_table;
  frame_table_global->frame_table_size = total_pmem_pages;

  // Allocate global data structures
  ready_queue = create_queue(); 
  running_process = malloc(sizeof(pte_t *));
  if (running_process == NULL) {
    TracePrintf(1, "KernelStart: Unable to allocate memory for running process. Halting.\n");
    Halt();
  }

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

  // set up our memory; note that we use the current kernel brk page in case the brk has changed
  int kernel_pageind = 0;
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
    }
    // After text, we give data RW permissions
    else if (kernel_pageind >= (kernel_data_start_page-1) && kernel_pageind < current_kernel_brk_page) {
      frame_table[kernel_pageind] = 1;
      kernel_page.valid = 1;
      kernel_page.prot = (PROT_READ | PROT_WRITE);
      kernel_page.pfn = kernel_pageind;
      region_0_page_table[kernel_pageind] = kernel_page;
    }
    // Until the stack, we add invalid pages to give the kernel the illusion of the whole reg. 0 memory space
    else if (kernel_pageind >=  current_kernel_brk_page && kernel_pageind < stack_start_page) {
      frame_table[kernel_pageind] = 1;
      kernel_page.valid = 0;
      kernel_page.pfn = kernel_pageind;
      region_0_page_table[kernel_pageind] = kernel_page;
    }
    // then we mark the stack as valid 
    else if (kernel_pageind >= stack_start_page && kernel_pageind < stack_end_page) {
      frame_table[kernel_pageind] = 1;
      kernel_page.valid = 1;
       kernel_page.prot = (PROT_READ | PROT_WRITE);
      kernel_page.pfn = kernel_pageind;
      region_0_page_table[kernel_pageind] = kernel_page;
    }
    // then we map the remainder of free physical frames
    else {
      frame_table[kernel_pageind] = 0;
    }

  }

  // update registers with region 0 page table
  WriteRegister(REG_PTBR0, (int) region_0_page_table);
  WriteRegister(REG_PTLR0, region_0_page_table_size);

  // turn on virtual memory permanently
  WriteRegister(REG_VM_ENABLE, 1);
  vmem_on = 1;

  TracePrintf(3, "========Region 0 Page Table Just After Setting VMEM======\n");
  for (int i = 0; i < region_0_page_table_size; i++) {
    if (region_0_page_table[i].valid) {
      TracePrintf(3, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                  VMEM_0_BASE + (i << PAGESHIFT),
                  VMEM_0_BASE + ((i+1) << PAGESHIFT)-1,
                  region_0_page_table[i].valid,
                  region_0_page_table[i].pfn
      );
    }
  }

  // TERMINALS
  if (init_kernel_tty_objects(NUM_TERMINALS) == ERROR) {
    TracePrintf(1, "KernelStart: Unable to initialize kernel tty objects. Halting.\n");
    Halt();
  }

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

  // set up region 1 page table
  int idle_stack_size = 2;
  pte_t idle_page;
  int page_table_reg_1_size = UP_TO_PAGE(VMEM_1_SIZE) >> PAGESHIFT;
  pte_t *region_1_page_table = malloc(sizeof(pte_t) * page_table_reg_1_size);
  if (region_1_page_table == NULL) {
    TracePrintf(1, "Error: could not allocate memory for region 1 page table. Halting.\n");
    Halt();
  }
  int first_possible_free_frame = 0;
  for (int ind=0; ind<page_table_reg_1_size; ind++) {
    // set everything under the stack as non-valid (since the text is in the kernel and
    // our loop shouldn't use any memory)
    idle_page.valid = 0;

    if (ind < page_table_reg_1_size - idle_stack_size) {
      region_1_page_table[ind] = idle_page;
    }
    // Here's the stack
    else {
      int new_frame_num = get_free_frame(
          frame_table_global->frame_table,
          frame_table_global->frame_table_size,
          first_possible_free_frame
          );
      if (new_frame_num == -1) {
        TracePrintf(1, "Unable to get a free frame for the idle process user stack!\n");
        return;
      }
      first_possible_free_frame = new_frame_num;
      idle_page.valid = 1;
      idle_page.prot = (PROT_READ | PROT_WRITE);
      idle_page.pfn = new_frame_num;
      region_1_page_table[ind] = idle_page;
    }
  }

  // modify the user context to act as text
  uctxt -> pc = &DoIdle;
  uctxt -> sp = (void *) VMEM_1_LIMIT - 8; // 8...it's a magic number

  //Create an idle pcb for this process
  int num_kernel_stack_pages = stack_end_page - stack_start_page;
  int pid = helper_new_pid(region_1_page_table);
  idle_process = allocate_pcb();
  if (idle_process == NULL) {
    TracePrintf(1, "Error: could not allocate memory for idle process. Halting.\n");
    Halt();
  }
  idle_process->pid = pid;
  idle_process = set_pcb_values(idle_process, pid, region_1_page_table, uctxt);
  for (int i=0; i<num_kernel_stack_pages; i++) {
    int stack_page_ind = (KERNEL_STACK_BASE >> PAGESHIFT) + i;
    idle_process->kernel_stack[i] = region_0_page_table[stack_page_ind];
  }

  //set it as the running process
  running_process = idle_process;

  // Create a pcb for init
  pcb_t *init_pcb = allocate_pcb();
  if (init_pcb == NULL) {
    TracePrintf(1, "Error: could not allocate memory for init process. Halting.\n");
    Halt();
  }
  int init_pid = helper_new_pid(init_pcb->region_1_page_table);
  init_pcb->pid = init_pid;
  add_to_queue(ready_queue, init_pcb);

  // update registers with the idle process's R1 page table
  WriteRegister(REG_PTBR1, (int) region_1_page_table);
  WriteRegister(REG_PTLR1, page_table_reg_1_size);

  // get the name of the default process, or a default
  char* name = cmd_args[0];
  if (name == NULL) {
    TracePrintf(1, "No init program specified, running default program!\n");
    name = "init";
  }

  //clone the current process into init_pcb
  TracePrintf(1, "Attempting to clone into init_pcb\n");
  int rc = clone_process(init_pcb);
  if (rc < 0) {
    TracePrintf(1, "Kernel boot code encountered an error and was unable to clone from idle into init_pcb.\n");
    Halt();
  }
  TracePrintf(1, "Cloned into init_pcb:\n");
  TracePrintf(1, "Process id is %d, idle is %d, init is %d\n", running_process->pid, idle_process->pid, init_pcb->pid);
  TracePrintf(1, "Process pc is %x, idle is %x, init is %x\n", running_process->uctxt->pc, idle_process->uctxt->pc, init_pcb->uctxt->pc);

  // make sure we only load the init process as init
  if (running_process->pid == init_pcb->pid) {
    // load the init process into the forked pcb
    TracePrintf(1, "Attempting to load program with name: %s\n", name);
    int load_program_rc;
    if ((load_program_rc = LoadProgram(name, cmd_args, init_pcb)) == -1 ||
        load_program_rc == -2
        ) {
      TracePrintf(1, "Loading the init process failed with exit code %d\n",
                  load_program_rc
                  );
      Halt();
    }
  } else {
    // make sure idle never gets in the queue
    is_idle = true;
  }

  // Set the uctxt to the desired place
  uctxt->pc = running_process->uctxt->pc;
  uctxt->sp = running_process->uctxt->sp;
  TracePrintf(1, "Process pc is %x, idle is %x, init is %x\n", running_process->uctxt->pc, idle_process->uctxt->pc, init_pcb->uctxt->pc);

  // return to userland
  TracePrintf(1, "Leaving KernelStart...\n");
  return; 
}

/*
* Set the kernel brk based on whether or not virtual memory is enabled. This will be a syscall used by malloc and other
* higher-level user/library dynamic allocation calls. In case of any error, the value ERROR is returned.
*/
int SetKernelBrk(void *addr) {
  TracePrintf(3, "SETKERNELBRK: Hit SetKernelBrk\n");
  // error out if we're provided an invalid address
  if (addr < _kernel_data_start) {
    TracePrintf(1, "SETKERNELBRK: Invalid address provided.\n");
    return ERROR;
  }
  // error out if we are trying to map the page right below the stack or the stack itself
  int stack_page_num = KERNEL_STACK_BASE >> PAGESHIFT;
  int addr_page = UP_TO_PAGE(addr - PMEM_BASE) >> PAGESHIFT;
  if (addr_page >= stack_page_num - 1) {
    TracePrintf(1, "SETKERNELBRK: Invalid address provided.\n");
    return ERROR;
  }
  // if vmem is not enabled, set the brk to the specified address above _kernel_origin_brk (hit by kernel malloc)
  if (!vmem_on) {
    TracePrintf(3, "SETKERNELBRK: VMem not enabled. Setting kernel brk page from %d to %d\n",
                current_kernel_brk_page, addr_page);
    current_kernel_brk_page = addr_page;
    return SUCCESS;
  }

  //otherwise, set the brk assuming the address is virtual 
  else {
    TracePrintf(3, "SETKERNELBRK: VMem enabled. Setting kernel brk from %d to %d\n",
                current_kernel_brk_page, addr_page);

    int first_possible_free_frame = 0;
    if (addr_page > current_kernel_brk_page) {
      TracePrintf(3, "SETKERNELBRK: Will try to find memory to allocate more frames\n");
      // error out if we don't have enough memory
      if (addr_page-current_kernel_brk_page > get_num_free_frames(frame_table_global->frame_table,
                                                                   frame_table_global->frame_table_size)
          ) {
        TracePrintf(1, "SETKERNELBRK: SetKernelBrk did not find enough memory for the whole malloc to succeed\n");
        return ERROR;
      }

      TracePrintf(3, "SETKERNELBRK: SetKernelBrk found enough memory for malloc to succeed\n");
      while (addr_page > current_kernel_brk_page) {
        int new_frame_num = get_free_frame(frame_table_global->frame_table,
                                           frame_table_global->frame_table_size,
                                           first_possible_free_frame);
        if (new_frame_num == -1) {
          TracePrintf(1, "SETKERNELBRK: SetKernelBrk was unable to allocate a new frame...\n");
          return ERROR;
        }
        region_0_page_table[current_kernel_brk_page].valid = 1;
        region_0_page_table[current_kernel_brk_page].prot = (PROT_READ | PROT_WRITE);
        region_0_page_table[current_kernel_brk_page].pfn = new_frame_num;
        current_kernel_brk_page++;
      }
      return SUCCESS;
    }
    else {
      // TODO -- does this code path ever execute?
      TracePrintf(3, "SETKERNELBRK: SetKernelBrk found that we don't need to allocate more frames\n");
      while (addr_page < current_kernel_brk_page) {
        TracePrintf(3, "SETKERNELBRK: Deleting a page from the page table...\n");
        int discard_frame_number = region_0_page_table[current_kernel_brk_page].pfn;
        frame_table_global->frame_table[discard_frame_number] = 0;
        region_0_page_table[current_kernel_brk_page].valid = 0;
        current_kernel_brk_page--;
      }
      return SUCCESS;
    }
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
