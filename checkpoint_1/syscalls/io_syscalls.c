#include <ykernel.h>
#include "io_syscalls.h"
#include "sync_syscalls.h"
#include "../memory/check_memory.h"
#include "../data_structures/tty.h"
#include "../data_structures/queue.h"
#include "../data_structures/lock.h"
#include "../kernel_utils.h"

extern pcb_t* running_process;
extern pcb_t* idle_process;
extern bool is_idle;
extern queue_t* ready_queue;
extern void *trap_handler[16];
extern pte_t *region_0_page_table;
extern tty_object_t *tty_objects[NUM_TERMINALS];
extern char tty_buffer[TTY_BUFFER_SIZE];

/* 
* Function to create new tty objects
* Called at boot time
*/
int init_kernel_tty_objects() {
  for (int i = 0; i<NUM_TERMINALS; i++) {
    tty_objects[i] = init_tty_object(i);
    if (tty_objects[i] == NULL) {
      TracePrintf(1, "Failed to allocate memory for tty global.\n");
      return ERROR;
    }
  }
}

int read_helper(tty_object_t* tty, char* buf, int len) {
  tty->reading = true;

  char next_char;
  int index = 0;
  while ((next_char = tty_buf_read_byte(tty)) != ERROR && index < len) {
    buf[index] = next_char;
    index++;
  }

  tty->reading = false;
  if (release(tty->read_lock->lock_id) == ERROR) {
    return ERROR;
  }

  return index;
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
  TracePrintf(1, "TtyRead: tty_id: %d, buf: %p, len: %d\n", tty_id, buf, len);

  // check the memory locations of this buffer
  if (check_memory(buf, len, false, true, false, false) == ERROR) {
    TracePrintf(1, "TtyRead: This buffer is not valid\n");
    return ERROR;
  }


  // get the data on the current tty object
  tty_object_t *tty = get_tty_object(tty_id);
  if (tty == NULL) {
    TracePrintf(1, "TtyRead: A TTY with Id %d does not exist\n", tty_id);
    return ERROR;
  }

  if (len == 0) {
    return SUCCESS;
  }

  // get the number of unconsumed chars
  int num_bytes_to_copy = tty->num_unconsumed_chars;

  TracePrintf(1, "Number unconsumed chars: %d\n", tty->num_unconsumed_chars);
  // if there are no bytes to copy, wait until there are bytes
  while (tty->num_unconsumed_chars == 0 || tty->reading) {
    // if there is no line available, block the calling process and wait for a line from terminal
    TracePrintf(1, "TtyRead: Blocking until we get chars\n");
    handle_CvarWait(tty->read_cvar->id, tty->read_lock->lock_id);
    TracePrintf(1, "TtyRead: Back from block on read\n");
  }

  TracePrintf(1, "TtyRead: There are supposedly chars to consume!\n");
  return read_helper(tty, buf, len);
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
  TracePrintf(1, "TtyWrite: Attempting to write bytes to a tty\n");
  // check the tty id
  if (tty_id < 0 || tty_id > (NUM_TERMINALS-1)) {
    TracePrintf(1, "TtyWrite: Invalid tty id %d\n", tty_id);
    return ERROR;
  }

  // check the memory locations of this buffer
  if (check_memory(buf, len, true, false, false, false) == ERROR) {
    TracePrintf(1, "TtyWrite: This buffer is not valid\n");
    return ERROR;
  }

  // get the data on the current tty object
  tty_object_t *tty = get_tty_object(tty_id);
  if (tty == NULL) {
    TracePrintf(1, "TtyWrite: A TTY with Id %d does not exist\n", tty_id);
    return ERROR;
  }

  // create a buffer of the same size as *buf, copy buf into it
  void* kernel_buf = malloc(len * sizeof (char));
  if (kernel_buf == NULL) {
    TracePrintf(1, "TtyWrite: write to tty %d failed because we couldn't allocate a kernel buf\n");
    return ERROR;
  }
  memcpy(kernel_buf, buf, len);

  // acquire the write lock
  if (acquire(tty->write_lock->lock_id) == ERROR) {
    TracePrintf(1, "TtyWrite: Unable to acquire write lock on tty %d\n", tty_id);
    return ERROR;
  }
  tty->writing_proc = running_process;

  // loop until there are no more unconsumed bytes in the buf
  int remaining_bytes = len;
  while (remaining_bytes > 0) {
    // unless this is the last line, transmit TERMINAL_MAX_LINE
    int bytes_to_transmit = TERMINAL_MAX_LINE;
    if (remaining_bytes < bytes_to_transmit) {
      bytes_to_transmit = remaining_bytes;
    }
    // make sure we're transmitting from the correct spot in the buffer
    TracePrintf(5, "Sending %d bytes to tty %d\n", bytes_to_transmit, tty_id);
    TtyTransmit(tty_id, kernel_buf+(len - remaining_bytes), bytes_to_transmit);

    remaining_bytes -= bytes_to_transmit;
    // swap to a different process while the tty is written
    install_next_from_queue(running_process, 1);
  }

  free(kernel_buf);

  if (release(tty->write_lock->lock_id) == ERROR) {
    return ERROR;
  }

  // return the number of bytes written
  return len;
}