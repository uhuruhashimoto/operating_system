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

int print_reg_0_page_table_global(int level);

int print_kernel_stack(int level);

int print_reg_1_page_table(pcb_t *process, int level);

int print_frame_table(int level);