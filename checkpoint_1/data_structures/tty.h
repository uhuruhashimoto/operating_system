//
// Created by smooth_operator on 10/19/22.
//

#ifndef CURRENT_CHUNGUS_TTY
#define CURRENT_CHUNGUS_TTY

#include <ykernel.h>
#include "queue.h"
#include "lock.h"
#include "cvar.h"
#include "stdbool.h"
#define MAX_BUFFER_LEN 100 //character storage in individual terminal
#define TTY_BUFFER_SIZE 200 //total kernel storage for terminals
/*
* This struct holds the metadata about each terminal, allowing us to read and write to it. Currently, we only allow a single
* process to read from a terminal at a time, and a single process to write to a terminal at a time.
*/
typedef struct tty_object {
  int id;
  lock_t *lock;
  cvar_t* cvar;
  bool in_use;
  char buf[MAX_BUFFER_LEN];
  int start_id;
  int end_id;
  int num_unconsumed_chars;
  int max_size;
  queue_t* blocked_reads; 
  queue_t* blocked_writes;
} tty_object_t;

tty_object_t *init_tty_object(int id);

tty_object_t *get_tty_object(int id);

/*
 * Gets the pcb_t* at head of tty queue, places it in ready queue, returns a pointer to it
 */
pcb_t* unblock_pcb_on_tty(int tty_id);

bool tty_buf_is_full(tty_object_t* tty);

bool tty_buf_is_empty(tty_object_t* tty);

char tty_buf_read_byte(tty_object_t* tty);

int tty_buf_write_byte(tty_object_t* tty, char byte);


#endif //CURRENT_CHUNGUS_TTY