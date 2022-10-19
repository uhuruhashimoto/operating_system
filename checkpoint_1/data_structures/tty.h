//
// Created by smooth_operator on 10/19/22.
//

#ifndef CURRENT_CHUNGUS_TTY
#define CURRENT_CHUNGUS_TTY

#include "queue.h"

typedef struct tty {
  int id;
  char buf[/* TODO -- buf length */];
  queue_t* blocked_reads;                               // blocked until
  queue_t* blocked_on_writing;
} tty_t;

bool is_full(int tty_id);

bool is_empty(int tty_id);

char read_byte(int tty_id);

int write_byte(int tty_id);

int block_pcb_on_pipe(int tty_id, pcb_t* process_block);

/********** unblock_pcb_on_pipe *************/
/*
 * Gets the pcb_t* at head of pipe, places it in ready queue, returns a pointer to it
 */
pcb_t* unblock_pcb_on_pipe(int tty_id);

#endif //CURRENT_CHUNGUS_TTY