#ifndef CURRENT_CHUNGUS_CHECK_MEMORY_H
#define CURRENT_CHUNGUS_CHECK_MEMORY_H

#include <ykernel.h>

/*
 * Checks to see if this memory is valid.
 * Assumes: the program will touch all bytes from mem_loc to mem_loc+mem_size
 *
 * Returns SUCCESS if the entire area of memory is touchable by the user (i.e., all in user space)
 * and has valid page entries through the entire section of memory.
 * Returns ERROR if this is not the case.
 */
int check_memory(void* mem_loc, unsigned int mem_size,
                 bool read_required, bool write_required, bool exec_required, bool r0_legal);

/*
 * Checks to see if this memory is valid.
 * Assumes: the program will touch all bytes from the start of the string to the first NULL byte
 *
 * Returns SUCCESS if the entire area of memory is touchable by the user (i.e., all in user space)
 * and has valid page entries through the entire section of memory.
 * Returns ERROR if this is not the case.
 */
int check_memory_string(char* mem_loc, bool read_required, bool write_required, bool exec_required, bool r0_legal);

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
int check_memory_string_array(char** mem_loc, bool read_required, bool write_required, bool exec_required, bool r0_legal);

#endif //CURRENT_CHUNGUS_CHECK_MEMORY_H