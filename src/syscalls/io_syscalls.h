// Specs are on Page 29 of Yalnix Manual

#ifndef CURRENT_CHUNGUS_IO_SYSCALL_HANDLERS
#define CURRENT_CHUNGUS_IO_SYSCALL_HANDLERS

#include <ykernel.h>
#include "../data_structures/tty.h"
#include "../data_structures/queue.h"

extern pcb_t* running_process;
extern pcb_t* idle_process;                                           // the special idle process; use when nothing is in ready queue
extern bool is_idle;                                                  // if is_idle, we won't put the process back on the ready queue
extern queue_t* ready_queue;
extern void *trap_handler[16];
extern pte_t *region_0_page_table;
extern tty_object_t *tty_objects[NUM_TERMINALS];
extern char tty_buffer[TTY_BUFFER_SIZE];

int init_kernel_tty_objects();

/*
 * Read the next line of input from terminal tty id, copying it into the buffer referenced by buf. The maximum
length of the line to be returned is given by len. The line returned in the buffer is not null-terminated.
If there are sufficient unread bytes already waiting, the call will return right away, with those.
Otherwise, the calling process is blocked until a line of input is available to be returned. If the length of the
next available input line is longer than len bytes, only the first len bytes of the line are copied to the calling
process, and the remaining bytes of the line are saved by the kernel for the next TtyRead (by this or another
process). If the length of the next available input line is shorter than len bytes, only as many bytes are copied
to the calling process as are available in the input line; On success, the number of bytes actually copied into the
calling process’s buffer is returned; in case of any error, the value ERROR is returned.
 */
int handle_TtyRead(int tty_id, void *buf, int len);

/*
 * Write the contents of the buffer referenced by buf to the terminal tty id. The length of the buffer in bytes
is given by len. The calling process is blocked until all characters from the buffer have been written on the
terminal. On success, the number of bytes written (len) is returned; in case of any error, the value ERROR is
returned.
Calls to TtyWrite for more than TERMINAL MAX LINE bytes should be supported.
 */
int handle_TtyWrite(int tty_id, void *buf, int len);

#endif //CURRENT_CHUNGUS_IO_SYSCALL_HANDLERS