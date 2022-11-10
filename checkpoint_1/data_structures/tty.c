#include <ykernel.h>
#include "lock.h"
#include "queue.h"
#include "stdbool.h"
#include "tty.h"
#include "cvar.h"

extern pcb_t* running_process;
extern pcb_t* idle_process;
extern bool is_idle;
extern queue_t* ready_queue;
extern void *trap_handler[16];
extern pte_t *region_0_page_table;
extern tty_object_t *tty_objects[NUM_TERMINALS];

tty_object_t *init_tty_object(int id) {
    tty_object_t *tty_obj = malloc(sizeof(tty_object_t));

    if (tty_obj == NULL) {
      TracePrintf(1, "INIT_TTY_OBJECT: Not enough space to allocate tty obj\n");
    }

    bzero(tty_obj->buf, MAX_BUFFER_LEN);
    tty_obj->id = id;
    tty_obj->in_use = false;
    tty_obj->lock = create_lock_any_id();
    tty_obj->cvar = create_cvar_any_id();
    tty_obj->num_unconsumed_chars = 0;
    tty_obj->max_size = MAX_BUFFER_LEN;
    tty_obj->start_id = 0;
    tty_obj->end_id = 0;
    tty_obj->blocked_reads = create_queue();
    tty_obj->blocked_writes = create_queue();

    if (tty_obj->blocked_reads == NULL || tty_obj->blocked_writes == NULL ||
        tty_obj->lock == NULL || tty_obj->cvar == NULL
    ) {
      TracePrintf(1, "INIT_TTY_OBJECT: One of the subobjects is NULL\n");
    }

    return tty_obj;
}

tty_object_t *get_tty_object(int id) {
    if (id <= NUM_TERMINALS-1) {
        return tty_objects[id];
    }
    return NULL;
}

/*
 * Gets the pcb_t* at head of tty queue, places it in ready queue, returns a pointer to it
 */
pcb_t* unblock_pcb_on_tty(int tty_id) {
    tty_object_t *tty_obj = get_tty_object(tty_id);
    pcb_t *pcb = remove_from_queue(tty_obj->blocked_reads);
    if (pcb != NULL) {
        add_to_queue(ready_queue, pcb);
    }
    return pcb;
}

bool tty_buf_is_full(tty_object_t* tty) {
  // check to see if the tty is full
  if (tty->num_unconsumed_chars == tty->max_size) {
    return true;
  }
  return false;
}

bool tty_buf_is_empty(tty_object_t* tty) {
  // check to see if the tty is empty
  if (tty->num_unconsumed_chars == 0) {
    return true;
  }
  return false;
}

char tty_buf_read_byte(tty_object_t* tty) {
  // returns ERROR if there is no byte to read (is_empty)
  if (tty_buf_is_empty(tty)) {
    return ERROR;
  }

  // reads a byte from the start_index of the buffer
  char byte = tty->buf[tty->start_id];

  // increments the start_index (rolls around if we exceed buf_size)
  tty->start_id = (tty->start_id + 1) % tty->max_size;
  tty->num_unconsumed_chars--;

  // returns the byte
  return byte;
}

int tty_buf_write_byte(tty_object_t* tty, char byte) {
  // returns ERROR if there is no more space in the buffer (is_full)
  if (tty_buf_is_full(tty)) {
    return ERROR;
  }

  // writes a byte at the end_index of the buffer
  tty->buf[tty->end_id] = byte;

  // increments the end_index (rolls around if necessary)
  tty->end_id = (tty->end_id + 1) % tty->max_size;
  tty->num_unconsumed_chars++;

  // returns 0
  return SUCCESS;
}


