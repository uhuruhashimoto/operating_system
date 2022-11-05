#include <ykernel.h>
#include <hardware.h>
#include "../kernel_start.h"

/*
 * Checks to see if this memory is valid.
 * Assumes: the program will touch all bytes from mem_loc to mem_loc+mem_size
 *
 * Returns SUCCESS if the entire area of memory is touchable by the user (i.e., all in user space)
 * and has valid page entries through the entire section of memory.
 * Returns ERROR if this is not the case.
 */
int check_memory(void* mem_loc, unsigned int mem_size) {
  int start_memory_loc_in_region_1 = ((int)mem_loc - VMEM_1_BASE);
  int end_memory_loc_in_region_1 = ((int)mem_loc + mem_size - VMEM_1_BASE);

  // are we in region 0?
  if (start_memory_loc_in_region_1 < 0 || end_memory_loc_in_region_1 < 0) {
    return ERROR;
  }

  int start_page_idx = start_memory_loc_in_region_1 >> PAGESHIFT;
  int end_page_idx = end_memory_loc_in_region_1 >> PAGESHIFT;

  // check all the page table entries between start and end page to see if they're valid
  for (int i = start_page_idx; i <= end_page_idx; i++) {
    if (!running_process->region_1_page_table[i]->valid) {
      return ERROR;
    }
  }

  return SUCCESS;
}