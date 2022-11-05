#include "pipe.h"
#include "queue.h"
#include "../kernel_start.h"
#include <ykernel.h>

/****************** UTILITY FUNCTIONS ***********************/

/*
 * Creates a new pipe with this id
 */
pipe_t* create_pipe(int pipe_id)
{
  pipe_t* pipe_obj = malloc(sizeof pipe_t);
  pipe_obj->pipe_id = pipe_id;
  pipe_obj->blocked_queue = create_queue();

  if (pipe_obj->blocked_queue == NULL) {
    free(pipe_obj);
    return NULL;
  }

  return pipe_obj;
}

/*
 * Returns the pipe with this ID, else NULL
 */
pipe_t* find_pipe(int pipe_id)
{
  pipe_t* next_pipe = pipes;
  while (next_pipe != NULL) {
    if (next_pipe->pipe_id == pipe_id) {
      return next_pipe;
    }
  }
  return NULL;
}

bool is_full(pipe_t* pipe)
{
  // check to see if the pipe is full
  if (pipe->cur_size == pipe->max_size) {
    return true;
  }
  return false;
}

bool pipe_is_empty(pipe_t* pipe)
{
  // check to see if the pipe is empty
  if (pipe->cur_size == 0) {
    return true;
  }
  return false;
}

char read_byte(pipe_t* pipe)
{
  // returns ERROR if there is no byte to read (is_empty)
  if (pipe_is_empty(pipe)) {
    return ERROR;
  }

  // reads a byte from the start_index of the buffer
  char byte = pipe->buf[pipe->start_id];

  // increments the start_index (rolls around if we exceed buf_size)
  pipe->start_id = (pipe->start_id + 1) % pipe->max_size;
  pipe->cur_size--;

  // returns the byte
  return byte;
}

int write_byte(pipe_t* pipe, char byte)
{
  // returns ERROR if there is no more space in the buffer (is_full)
  if (is_full(pipe)) {
    return ERROR;
  }

  // writes a byte at the end_index of the buffer
  pipe->buf[pipe->end_id] = byte;

  // increments the end_index (rolls around if necessary)
  pipe->end_id = (pipe->end_id + 1) % pipe->max_size;
  pipe->cur_size++;

  // returns 0
  return SUCCESS;
}

int block_pcb_on_pipe_read(pipe_t* pipe, pcb_t* process_block)
{
  // place the pcb on the queue associated with this pipe
  return add_to_queue(pipe->blocked_read_queue, process_block);
}

int block_pcb_on_pipe_write(pipe_t* pipe, pcb_t* process_block)
{
  // place the pcb on the queue associated with this pipe
  return add_to_queue(pipe->blocked_write_queue, process_block);
}

/********** unblock_pcb_on_pipe *************/
/*
 * Gets the pcb_t* at head of pipe, places it in ready queue, returns a pointer to it
 */
pcb_t* unblock_pcb_on_pipe(pipe_t* pipe)
{
  // gets the next pcb in the blocked queue of the pipe
  pcb_t* next_pcb = remove_from_queue(pipe->blocked_queue);

  // adds the pcb to the ready queue
  add_to_queue(ready_queue, next_pcb);

  // returns the pcb
  return next_pcb;
}

/***************** END UTILITY FUNCTIONS *********************/