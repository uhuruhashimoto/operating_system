#include "ipc_syscalls.h"
#include "stdbool.h"
#include "../data_structures/pcb.h"
#include "../data_structures/pipe.h"
#include "../data_structures/queue.h"
#include "../data_structures/lock.h"
#include "../kernel_start.h"
#include "../kernel_utils.h"
#include "../memory/check_memory.h"

/*
 * Create a new pipe; save its identifier at *pipe idp. (See the header files for the length of the pipe’s internal
buffer.) In case of any error, the value ERROR is returned.
 */
int handle_PipeInit(int *pipe_idp)
{
  TracePrintf(1, "HANDLE_PIPE_INIT: attempting to create a new pipe\n");

  if (check_memory(pipe_idp, sizeof (int), false, true, false, false) == ERROR) {
    return ERROR;
  }

  unsigned int next_id = ++max_pipe_id;

  if (next_id > max_possible_pipe_id) {
    TracePrintf(1, "Run out of ID space to allocate more pipes\n");
    return ERROR;
  }

  // create a new pipe
  pipe_t* new_pipe = create_pipe(next_id);
  if (new_pipe == NULL) {
    TracePrintf(1, "HANDLE_PIPE_INIT: failed to create a new pipe\n");
    return ERROR;
  }

  // stick it at the head of the pipe linked list
  pipe_t* prev_ll = pipes;
  pipes = new_pipe;
  new_pipe->next_pipe = prev_ll;
  if (prev_ll != NULL) {
    prev_ll->prev_pipe = new_pipe;
  }

  pipe_idp[0] = next_id;
  return SUCCESS;
}

/*
 * Read len consecutive bytes from the named pipe into the buffer starting at address buf, following the standard
semantics:
– If the pipe is empty, then block the caller.
– If the pipe has plen ≤ len unread bytes, give all of them to the caller and return.
– If the pipe has plen > len unread bytes, give the first len bytes to caller and return. Retain the unread
plen − len bytes in the pipe.
In case of any error, the value ERROR is returned. Otherwise, the return value is the number of bytes read.
 */
int handle_PipeRead(int pipe_id, void *buf, int len)
{
  TracePrintf(1, "HANDLE_PIPE_READ: Reading from a pipe with id %d\n", pipe_id);

  if (check_memory(buf, (unsigned int) len, false, true, false, false) == ERROR) {
    return ERROR;
  }

  pipe_t* found_pipe = find_pipe(pipe_id);
  if (found_pipe == NULL) {
    TracePrintf(1, "HANDLE_PIPE_READ: Unable to find a pipe with id %d\n", pipe_id);
    return ERROR;
  }
  TracePrintf(1, "HANDLE_PIPE_READ: Acquiring read lock on a pipe with id %d\n", pipe_id);
  // acquire the lock for the pipe
  acquire(found_pipe->read_lock->lock_id);
  TracePrintf(1, "HANDLE_PIPE_READ: Acquired read lock on a pipe with id %d\n", pipe_id);

  // if the pipe is empty, block the caller:
  // this should only be hit once
  while (pipe_is_empty(found_pipe)) {
    TracePrintf(1, "HANDLE_PIPE_READ: Pipe with id %d is empty; blocking process\n", pipe_id);
    //   put the caller in the blocked queue of the pipe
    block_pcb_on_pipe_read(found_pipe, running_process);
    //   swap a new process into the ready slot for execution
    install_next_from_queue(running_process, 1);
  }

  TracePrintf(1, "HANDLE_PIPE_READ: Reading bytes from pipe with id %d\n", pipe_id);

  // read bytes from the pipe into *buf, until we either hit len or the number of bytes in found_pipe
  int buf_loc = 0;
  while (found_pipe->cur_size > 0 && buf_loc < len) {
    char new_char = read_byte(found_pipe);
    ((char *)buf)[buf_loc] = new_char;
    buf_loc++;
  }

  TracePrintf(1, "HANDLE_PIPE_READ: Finished reading bytes from pipe with id %d\n", pipe_id);

  // release the lock for the pipe
  release(found_pipe->read_lock->lock_id);

  TracePrintf(1, "HANDLE_PIPE_READ: Released the lock\n");

  pcb_t* new_writer = remove_from_queue(found_pipe->blocked_write_queue);
  TracePrintf(1, "HANDLE_PIPE_READ: Removed from queue\n");
  if (new_writer != NULL) {
    add_to_queue(ready_queue, new_writer);
  }

  return buf_loc+1;
}

/*
 * Write the len bytes starting at buf to the named pipe. (As the pipe is a FIFO buffer, these bytes should be
appended to the sequence of unread bytes currently in the pipe.) Return as soon as you get the bytes into the
buffer. In case of any error, the value ERROR is returned. Otherwise, return the number of bytes written.
 */
int handle_PipeWrite(int pipe_id, void *buf, int len)
{
  TracePrintf(1, "HANDLE_PIPE_WRITE: Writing %d bytes to pipe with id %d\n", len, pipe_id);

  if (check_memory(buf, (unsigned int) len, true, false, false, false) == ERROR) {
    return ERROR;
  }

  pipe_t* found_pipe = find_pipe(pipe_id);
  if (found_pipe == NULL) {
    TracePrintf(1, "HANDLE_PIPE_WRITE: Unable to find a pipe with id %d\n", pipe_id);
    return ERROR;
  }
  // acquire the lock for the pipe
  acquire(found_pipe->write_lock->lock_id);

  int buf_index = 0;
  int num_left = len;
  while (num_left > 0) {
    // calculate the number of bytes left to write
    //     checks to see how much free space the pipe buffer has
    int next_byte_cluster_size = found_pipe->max_size - found_pipe->cur_size;
    if (num_left < next_byte_cluster_size) {
      next_byte_cluster_size = num_left;
    }

    num_left -= next_byte_cluster_size;
    //     write all bytes we can into the pipe buffer, checking return code
    int rc;
    while (next_byte_cluster_size > 0) {
      char next_byte = ((char *)buf)[buf_index];
      rc = write_byte(found_pipe, next_byte);
      if (rc == ERROR) {
        return ERROR;
      }
      next_byte_cluster_size--;
      buf_index++;
    }

    if (num_left > 0) {
      TracePrintf(1, "HANDLE_PIPE_WRITE: Pipe with id %d is full; blocking process\n", pipe_id);
      //   put the caller in the blocked queue of the pipe
      block_pcb_on_pipe_write(found_pipe, running_process);
      //   swap a new process into the ready slot for execution
      install_next_from_queue(running_process, 1);
    }
  }

  // release the lock for the pipe
  release(found_pipe->write_lock->lock_id);

  // unblock things that were blocked on read/write
  pcb_t* new_reader = remove_from_queue(found_pipe->blocked_read_queue);
  if (new_reader != NULL) {
    add_to_queue(ready_queue, new_reader);
  }

  return len;
}

/*
* Kill pipe by pipe id, and any queued children waiting for pipe input. If necessary, 
* we could specify a kill/don't kill option in our input args.
 *
 * kill_children = 0  --> don't kill -- these processes are put back in the ready queue
 *                                      which may have weird side effects!
 * kill_children = 1  --> do kill (default)
*/
int handle_PipeKill(int pipe_id, int kill_children) {
  TracePrintf(1, "HANDLE_PIPE_KILL: Attempting to delete a pipe with id %d\n", pipe_id);

  pipe_t* found_pipe = find_pipe(pipe_id);
  if (found_pipe == NULL) {
    TracePrintf(1, "HANDLE_PIPE_KILL: Unable to find a pipe with id %d\n", pipe_id);
    return ERROR;
  }

  // kill children, or put them in ready queue
  // clear the read-blocked children
  pcb_t* next_child = remove_from_queue(found_pipe->blocked_read_queue);
  while (next_child != NULL) {
    if (kill_children == 1) {
      delete_process(next_child, ERROR, false);
    }
    else {
      add_to_queue(ready_queue, next_child);
    }
    next_child = remove_from_queue(found_pipe->blocked_read_queue);
  }

  // clear the write-blocked children
  next_child = remove_from_queue(found_pipe->blocked_write_queue);
  while (next_child != NULL) {
    if (kill_children == 1) {
      delete_process(next_child, ERROR, false);
    }
    else {
      add_to_queue(ready_queue, next_child);
    }
    next_child = remove_from_queue(found_pipe->blocked_write_queue);
  }

  // stitch the pipe list together
  if (found_pipe == pipes) {
    pipes = found_pipe->next_pipe;
  }
  if (found_pipe->prev_pipe != NULL) {
    found_pipe->prev_pipe->next_pipe = found_pipe->next_pipe;
  }
  if (found_pipe->next_pipe != NULL) {
    found_pipe->next_pipe->prev_pipe = found_pipe->prev_pipe;
  }

  // delete the pipe
  delete_pipe(found_pipe);

  return SUCCESS;
}