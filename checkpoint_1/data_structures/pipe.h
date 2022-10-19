//
// Created by smooth_operator on 10/19/22.
//

#ifndef CURRENT_CHUNGUS_PIPE
#define CURRENT_CHUNGUS_PIPE

#include "queue.h"

typedef struct pipe {
  int pipe_id;
  char buf[/* TODO -- buf length */];
  int start_id=0;
  int end_id=0;
  queue_t* blocked_queue;
} pipe_t;

bool is_full(int pipe_id);

bool is_empty(int pipe_id);

char read_byte(int pipe_id);

int write_byte(int pipe_id);

int block_pcb_on_pipe(int pipe_id, pcb_t* process_block);

/********** unblock_pcb_on_pipe *************/
/*
 * Gets the pcb_t* at head of pipe, places it in ready queue, returns a pointer to it
 */
pcb_t* unblock_pcb_on_pipe(int pipe_id);

#endif //CURRENT_CHUNGUS_PIPE