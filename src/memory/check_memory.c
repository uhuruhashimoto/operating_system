#include <ykernel.h>
#include <hardware.h>
#include "../kernel_start.h"

/*
 *
We're no strangers to love
You know the rules and so do I
A full commitment's what I'm thinking of
You wouldn't get this from any other guy

I just wanna tell you how I'm feeling
Gotta make you understand

Never gonna give you up
Never gonna let you down
Never gonna run around and desert you
Never gonna make you cry
Never gonna say goodbye
Never gonna tell a lie and hurt you

We've known each other for so long
Your heart's been aching, but
You're too shy to say it
Inside, we both know what's been going on
We know the game and we're gonna play it

And if you ask me how I'm feeling
Don't tell me you're too blind to see

Never gonna give you up
Never gonna let you down
Never gonna run around and desert you
Never gonna make you cry
Never gonna say goodbye
Never gonna tell a lie and hurt you

Never gonna give you up
Never gonna let you down
Never gonna run around and desert you
Never gonna make you cry
Never gonna say goodbye
Never gonna tell a lie and hurt you

(Ooh, give you up)
(Ooh, give you up)
Never gonna give, never gonna give
(Give you up)
Never gonna give, never gonna give
(Give you up)

We've known each other for so long
Your heart's been aching, but
You're too shy to say it
Inside, we both know what's been going on
We know the game and we're gonna play it

I just wanna tell you how I'm feeling
Gotta make you understand

Never gonna give you up
Never gonna let you down
Never gonna run around and desert you
Never gonna make you cry
Never gonna say goodbye
Never gonna tell a lie and hurt you

Never gonna give you up
Never gonna let you down
Never gonna run around and desert you
Never gonna make you cry
Never gonna say goodbye
Never gonna tell a lie and hurt you

Never gonna give you up
Never gonna let you down
Never gonna run around and desert you
Never gonna make you cry
Never gonna say goodbye
Never gonna tell a lie and hurt you
 */

int check_memory_r0(void* mem_loc, unsigned int mem_size, bool read_required, bool write_required, bool exec_required);

int check_page(int prot, bool read_required, bool write_required, bool exec_required) {
  TracePrintf(1, "Prot: %d, read %d, write %d, exec %d", prot, read_required, write_required, exec_required);

  bool is_read_enabled = false;
  bool is_write_enabled = false;
  bool is_exec_enabled = false;
  if (prot == (PROT_READ)) {
    is_read_enabled = true;
  } else if (prot == (PROT_WRITE)) {
    is_write_enabled = true;
  } else if (prot == (PROT_EXEC)) {
    is_exec_enabled = true;
  } else if (prot == (PROT_READ | PROT_WRITE)) {
    is_read_enabled = true;
    is_write_enabled = true;
  } else if (prot == (PROT_READ | PROT_EXEC)) {
    is_read_enabled = true;
    is_exec_enabled = true;
  } else if (prot == (PROT_WRITE | PROT_EXEC)) {
    is_write_enabled = true;
    is_exec_enabled = true;
  } else if (prot == (PROT_READ | PROT_WRITE | PROT_EXEC)) {
    is_read_enabled = true;
    is_write_enabled = true;
    is_exec_enabled = true;
  }

  if ((read_required && !is_read_enabled) || (write_required && !is_write_enabled) || (exec_required && !is_exec_enabled)) {
    return ERROR;
  }
  return SUCCESS;
}

/*
 * Checks to see if this memory is valid.
 * Assumes: the program will touch all bytes from mem_loc to mem_loc+mem_size
 *
 * Returns SUCCESS if the entire area of memory is touchable by the user (i.e., all in user space)
 * and has valid page entries through the entire section of memory.
 * Returns ERROR if this is not the case.
 */
int check_memory(void* mem_loc, unsigned int mem_size,
                 bool read_required, bool write_required, bool exec_required, bool r0_legal) {
  // fail immediately if we're reading bytes above region 1 or region 0
  // we cannot examine these memory addresses directly.
  if ((unsigned int) mem_loc > VMEM_1_BASE+VMEM_1_SIZE) {
    return ERROR;
  }

  int start_memory_loc_in_region_1 = ((unsigned int)mem_loc - VMEM_1_BASE);
  // note that a 4-byte object goes 0x00, 0x01, 0x02, 0x03   (i.e., end=start+size-1)
  int end_memory_loc_in_region_1 = ((unsigned int)mem_loc + mem_size - 1- VMEM_1_BASE);

  // are we in region 0?
  if ((start_memory_loc_in_region_1 < 0 || end_memory_loc_in_region_1 < 0)) {
    if (r0_legal) {
      return check_memory_r0(mem_loc, mem_size, read_required, write_required, exec_required);
    }
    TracePrintf(1, "CHECK_MEMORY: You aren't allowed to access kernel memory on this operation\n");
    return ERROR;
  }

  int start_page_idx = start_memory_loc_in_region_1 >> PAGESHIFT;
  int end_page_idx = end_memory_loc_in_region_1 >> PAGESHIFT;

  TracePrintf(5, "Addr: %x, Start page: %d, end page: %d\n", mem_loc, start_page_idx, end_page_idx);

  // check all the page table entries between start and end page to see if they're valid
  for (int i = start_page_idx; i <= end_page_idx; i++) {
    if (!running_process->region_1_page_table[i].valid) {
      TracePrintf(1, "CHECK_MEMORY: Found invalid R1 page!\n");
      return ERROR;
    }

    if (check_page(running_process->region_1_page_table[i].prot, read_required, write_required, exec_required) == ERROR) {
      TracePrintf(1, "CHECK_MEMORY: Found R1 page with the wrong permissions!\n");
      return ERROR;
    }
  }

  return SUCCESS;
}

/*
 * A special case for r0.
 *
 * TODO -- items on the kernel stack
 */
int check_memory_r0(void* mem_loc, unsigned int mem_size, bool read_required, bool write_required, bool exec_required) {
  int start_memory_loc_in_region_0 = ((unsigned int)mem_loc - VMEM_0_BASE);
  // note that a 4-byte object goes 0x00, 0x01, 0x02, 0x03   (i.e., end=start+size-1)
  int end_memory_loc_in_region_0 = ((unsigned int)mem_loc + mem_size - 1 - VMEM_0_BASE);

  int start_page_idx = start_memory_loc_in_region_0 >> PAGESHIFT;
  int end_page_idx = end_memory_loc_in_region_0 >> PAGESHIFT;

  TracePrintf(1, "Addr: %x, Start page: %d, end page: %d\n", mem_loc, start_page_idx, end_page_idx);

  // check all the page table entries between start and end page to see if they're valid
  for (int i = start_page_idx; i <= end_page_idx; i++) {
    if (!region_0_page_table[i].valid) {
      TracePrintf(1, "CHECK_MEMORY: Found invalid R0 page!\n");
      return ERROR;
    }

    if (check_page(region_0_page_table[i].prot, read_required, write_required, exec_required) == ERROR) {
      TracePrintf(1, "CHECK_MEMORY: Found R0 page with the wrong permissions!\n");
      return ERROR;
    }
  }
  TracePrintf(1, "Success!\n");
  return SUCCESS;
}

/*
 * Checks to see if this memory is valid.
 * Assumes: the program will touch all bytes from the start of the string to the first NULL byte
 *
 * Returns SUCCESS if the entire area of memory is touchable by the user (i.e., all in user space)
 * and has valid page entries through the entire section of memory.
 * Returns ERROR if this is not the case.
 */
int check_memory_string(char* mem_loc, bool read_required, bool write_required, bool exec_required, bool r0_legal) {
  bool string_terminated = false;
  char* current_scan_loc = mem_loc;
  TracePrintf(1, "CHECK_MEMORY_STRING: Checking a string %x\n", mem_loc);
  // scan until we hit invalid memory, or until we hit a NULL byte
  while (check_memory(current_scan_loc, sizeof(char), read_required, write_required, exec_required, r0_legal) != ERROR) {
    TracePrintf(1, "In next place\n");
    char car = current_scan_loc[0];
    TracePrintf(1, "%c\n", car);
    TracePrintf(1, "In next place\n");

    // if we hit '\0', we've found the end of the string
    if (current_scan_loc[0] == '\0') {
      string_terminated = true;
      break;
    }
    current_scan_loc++;
  }
  if (string_terminated) {
    return SUCCESS;
  }
  else {
    return ERROR;
  }
}

/*
 * Checks to see if this memory is valid.
 * Assumes: the program will touch all strings from the 0th string to the first NULL pointer
 *  We will check the boolean flags on all pointer locations in the string array
 *  We will check the boolean flags on all strings in the string array
 *
 * Returns SUCCESS if the entire area of memory is touchable by the user (i.e., all in user space)
 * and has valid page entries through the entire section of memory.
 * Returns ERROR if this is not the case.
 */
int check_memory_string_array(char** mem_loc, bool read_required, bool write_required, bool exec_required, bool r0_legal) {
  bool array_terminated = false;
  char** current_scan_loc = mem_loc;
  // scan until we hit invalid memory, or until we hit a NULL byte
  while (check_memory(current_scan_loc, sizeof(char*), read_required, write_required, exec_required, r0_legal) != ERROR) {

    // if we hit NULL, we've found the end of the array
    if (current_scan_loc[0] == NULL) {
      array_terminated = true;
      break;
    }
    // we check each string in the array for validity
    if (check_memory_string(current_scan_loc[0], read_required, write_required, exec_required, r0_legal) == ERROR) {
      break;
    }
    current_scan_loc++;

    TracePrintf(5, "CHECK_MEMORY_STRING_ARRAY: About to check next array location\n");
  }
  if (array_terminated) {
    TracePrintf(1, "CHECK_MEMORY_STRING_ARRAY: This address should be legal...\n");
    return SUCCESS;
  }
  else {
    return ERROR;
  }
}