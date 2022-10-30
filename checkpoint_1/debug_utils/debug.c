#include <ykernel.h>
#include "../kernel_start.h"
#include "../data_structures/pcb.h"
#include "../data_structures/queue.h"
#include "../data_structures/frame_table.h"

extern frame_table_struct_t *frame_table_global;
extern pcb_t* running_process;
extern pcb_t* idle_process;                                           // the special idle process; use when nothing is in ready queue
extern bool is_idle;                                                  // if is_idle, we won't put the process back on the ready queue
extern queue_t* ready_queue;
extern void *trap_handler[16];
extern pte_t *region_0_page_table;

// print current page table global
void print_reg_0_page_table(int level) {
    int region_0_page_table_size = UP_TO_PAGE(VMEM_0_SIZE) >> PAGESHIFT; 
    TracePrintf(level, "=====Region 0 Page Table (%d pages)=====\n", region_0_page_table_size);
    for (int i = 0; i < region_0_page_table_size; i++) {
        if (region_0_page_table[i].valid) {
            TracePrintf(level, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                        VMEM_0_BASE + (i << PAGESHIFT),
                        VMEM_0_BASE + ((i+1) << PAGESHIFT)-1,
                        region_0_page_table[i].valid,
                        region_0_page_table[i].pfn
            );
        }
    }
}

// print current kernel stack (from page table)
void print_kernel_stack(int level) {
    int stack_start_page = UP_TO_PAGE(KERNEL_STACK_BASE - PMEM_BASE) >> PAGESHIFT;
    int stack_end_page = UP_TO_PAGE(KERNEL_STACK_LIMIT - PMEM_BASE) >> PAGESHIFT;

    TracePrintf(level, "=====Current Kernel Stack=====\n");
    for (int i=stack_start_page; i<stack_end_page; i++) {
       TracePrintf(level, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                    VMEM_0_BASE + (i << PAGESHIFT),
                    VMEM_0_BASE + ((i+1) << PAGESHIFT)-1,
                    region_0_page_table[i].valid,
                    region_0_page_table[i].pfn
        ); 
    }
    
}

void print_region_1_page_table_contents(pcb_t *process, int level) {
    pte_t *region_1_page_table = process->region_1_page_table;
    int region_1_page_table_size = UP_TO_PAGE(VMEM_1_SIZE) >> PAGESHIFT;
    TracePrintf(level, "=====Region 1 Page Table Contents for pid %d (%d pages)=====\n", process->pid, region_1_page_table_size);
    for (int i = 0; i < region_1_page_table_size; i++) {
        if (region_1_page_table[i].valid) {
            int *addr = (int *) (VMEM_1_BASE + (i << PAGESHIFT)); 
            TracePrintf(level, "Addr: %x, Pfn: %d, Bytes: %x\n",
                        addr,
                        region_1_page_table[i].pfn,
                        *addr
            );
        } 
    } 
}

// print region 1 page table for a given process
void print_reg_1_page_table(pcb_t *process, int level) {
    pte_t *region_1_page_table = process->region_1_page_table;
    int region_1_page_table_size = UP_TO_PAGE(VMEM_1_SIZE) >> PAGESHIFT;
    TracePrintf(level, "=====Region 1 Page Table for pid %d (%d pages)=====\n", process->pid, region_1_page_table_size);
    for (int i = 0; i < region_1_page_table_size; i++) {
        if (region_1_page_table[i].valid) {
            TracePrintf(level, "Addr: %x to %x, Valid: %d, Pfn: %d\n",
                        VMEM_1_BASE + (i << PAGESHIFT),
                        VMEM_1_BASE + ((i+1) << PAGESHIFT)-1,
                        region_1_page_table[i].valid,
                        region_1_page_table[i].pfn
            );
        } 
    }
}

// print the frame table (all frames)
void print_frame_table(int level) {
    for (int i=0; i<frame_table_global->frame_table_size; i++) {
        TracePrintf(level, "Frame %d: %d\n", i, frame_table_global[i]);
    }
}