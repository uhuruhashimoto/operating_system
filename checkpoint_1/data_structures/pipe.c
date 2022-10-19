#include "pipe.h"

/****************** UTILITY FUNCTIONS ***********************/

bool is_full(int pipe_id)
{
  // check to see if the pipe is full -- i.e., the end index + 1 == the start index
}

bool is_empty(int pipe_id)
{
  // check to see if the pipe is empty -- i.e., the start and end indices of the buffer are the same
}

char read_byte(int pipe_id)
{
  // returns ERROR if there is no byte to read (is_empty)
  // reads a byte from the start_index of the buffer
  // increments the start_index (rolls around if we exceed buf_size)
  // returns the byte
}

int write_byte(int pipe_id)
{
  // returns ERROR if there is no more space in the buffer (is_full)
  // writes a byte one past the end_index of the buffer (rolls around if necessary)
  // increments the end_index (rolls around if necessary)
  // returns 0
}

int block_pcb_on_pipe(int pipe_id, pcb_t* process_block)
{
  // place the pcb on the queue associated with this pipe
  // return SUCCESS or ERROR
}

/********** unblock_pcb_on_pipe *************/
/*
 * Gets the pcb_t* at head of pipe, places it in ready queue, returns a pointer to it
 */
pcb_t* unblock_pcb_on_pipe(int pipe_id)
{
  // gets the next pcb in the blocked queue of the pipe
  // adds the pcb to the ready queue
  // returns the pcb
}

/***************** END UTILITY FUNCTIONS *********************/