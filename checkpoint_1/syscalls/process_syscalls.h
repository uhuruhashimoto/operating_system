//
// Created by smooth_operator on 10/8/22.
// Specs are on Page 29 of Yalnix Manual
//

#ifndef CURRENT_CHUNGUS_PROCESS_SYSCALL_HANDLERS
#define CURRENT_CHUNGUS_PROCESS_SYSCALL_HANDLERS
#include <ykernel.h>

/*
 * Fork the process and create a new, separate address space
 */
int handle_Fork(void);

/*
 * Overwrite the address space of the old process with that of a new process
 * filename is the process to be executed
 * argvec is a null-terminated list of pointers to argument strings
 */
int handle_Exec(char *filename, char **argvec);

/*
 * Terminates the process, saving status for later retrieval by the parent
 * all other resources are freed
 * Orphaned processes will not store the status, free everything instead
 */
void handle_Exit(int status);

/*
 * Collect PID and exit status of some child process
 * returns immediately if child is already dead
 * ERROR if calling process has no remaining children, alive or dead
 * otherwise waits until next child exits
 */
int handle_Wait(int *status_ptr);

/*
 * Gets the PID of the calling process
 */
int handle_GetPid(void);

/*
 * Sets the brk
 * ERROR on failure, 0 on sucess
 */
int handle_Brk(void *addr);

/*
 * The calling process is blocked until at least clock ticks clock interrupts have occurred after the call. Upon
completion of the delay, the value 0 is returned.
If clock ticks is 0, return is immediate. If clock ticks is less than 0, time travel is not carried out, and
ERROR is returned instead.
 */
int handle_Delay(int clock_ticks);

#endif //CURRENT_CHUNGUS_PROCESS_SYSCALL_HANDLERS