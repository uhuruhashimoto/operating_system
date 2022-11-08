#include <ykernel.h>
#include "io_syscalls.h"
#include "../data_structures/tty.h"
#include "../data_structures/queue.h"
#include "../data_structures/lock.h"

extern pcb_t* running_process;
extern pcb_t* idle_process;
extern bool is_idle;
extern queue_t* ready_queue;
extern void *trap_handler[16];
extern pte_t *region_0_page_table;
extern tty_object_t *tty_objects[NUM_TERMINALS];

/* 
* Function to create new tty objects
* Called at boot time
*/
int init_kernel_tty_objects() {
  for (int i = 0; i<NUM_TERMINALS; i++) {
    tty_objects[i] = init_tty_object(i);
  }
}

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
int handle_TtyRead(int tty_id, void *buf, int len)
{
  // if there is no available line on the terminal, block the calling process and wait for a line from terminal

  // copy line from terminal tty_id to buf, of max size len
  // save any remaining bytes for later use by the kernel

  // return the number of bytes copied into buf
}

/*
 * Write the contents of the buffer referenced by buf to the terminal tty id. The length of the buffer in bytes
is given by len. The calling process is blocked until all characters from the buffer have been written on the
terminal. On success, the number of bytes written (len) is returned; in case of any error, the value ERROR is
returned.
Calls to TtyWrite for more than TERMINAL MAX LINE bytes should be supported.
 */
int handle_TtyWrite(int tty_id, void *buf, int len)
{
  // get the data on the current tty object
  tty_object_t *tty = tty_objects[tty_id];

  // block the calling process
  pcb_t *current_process = running_process;
  add_to_queue(tty->blocked_writes, current_process);
  
  // wait to acquire the write_lock on the tty
  while (!tty->in_use) {
    if (acquire(tty->lock->lock_id) == ERROR) {
      return ERROR;
    }
  }
  tty->in_use = true;
  if (release(tty->lock->lock_id) == ERROR) {
    return ERROR;
  }

  // loop until there are no more unconsumed bytes in the buf
  //  write TERMINAL_MAX_LINE bytes from the buf to the terminal
  int num_lines = len / TERMINAL_MAX_LINE;
  for (int i = 0; i < num_lines; i++) {
    TtyTransmit(tty_id, buf, TERMINAL_MAX_LINE);
  }

  // Update tty metadata
  if (acquire(tty->lock->lock_id) == ERROR) {
    return ERROR;
  }
  tty->in_use = false;
  if (release(tty->lock->lock_id) == ERROR) {
    return ERROR;
  }

  // unblock the calling process (place on the ready queue)
  if (tty->blocked_writes->head == current_process) {
    remove_from_queue(tty->blocked_writes);
  }
  add_to_queue(ready_queue, current_process);

  // return the number of bytes written
  return len;
}