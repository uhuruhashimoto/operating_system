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
      return NULL;
    }

    bzero(tty_obj->buf, MAX_BUFFER_LEN);
    tty_obj->id = id;
    tty_obj->reading = false;
    tty_obj->writing = false;
    tty_obj->read_lock = create_lock_any_id();
    tty_obj->read_cvar = create_cvar_any_id();
    tty_obj->write_lock = create_lock_any_id();
    tty_obj->write_cvar = create_cvar_any_id();
    tty_obj->writing_proc = NULL;
    tty_obj->num_unconsumed_chars = 0;
    tty_obj->max_size = MAX_BUFFER_LEN;
    tty_obj->start_id = 0;
    tty_obj->end_id = 0;

    if (tty_obj->read_lock == NULL || tty_obj->read_cvar == NULL ||
        tty_obj->write_lock == NULL || tty_obj->write_cvar == NULL
    ) {
      TracePrintf(1, "INIT_TTY_OBJECT: One of the subobjects is NULL\n");
      return NULL;
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
 * Checks if the terminal buffer is full
 */
bool tty_buf_is_full(tty_object_t* tty) {
  if (tty == NULL) {
    TracePrintf(1, "TTY_BUF_IS_FULL: tty is NULL\n");
    return true;
  }
  // check to see if the tty is full
  if (tty->num_unconsumed_chars == tty->max_size) {
    return true;
  }
  return false;
}

/*
 * Checks if the terminal buffer is empty
 */
bool tty_buf_is_empty(tty_object_t* tty) {
  if (tty == NULL) {
    TracePrintf(1, "TTY_BUF_IS_EMPTY: tty is NULL\n");
    return true;
  }
  // check to see if the tty is empty
  if (tty->num_unconsumed_chars == 0) {
    return true;
  }
  return false;
}

/*
 * Read a byte from the tty buf
 * returns the byte if it exists, ERROR upon error
 */
char tty_buf_read_byte(tty_object_t* tty) {
  // Null checks for input
  if (tty == NULL) {
    TracePrintf(1, "TTY_BUF_READ_BYTE: tty is NULL\n");
    return ERROR;
  }
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
  // Null checks for input
  if (tty == NULL) {
    TracePrintf(1, "TTY_BUF_READ_BYTE: tty is NULL\n");
    return ERROR;
  }
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


