#include <ykernel.h>
#include "queue.h"
#include "tty.h"

tty_object_t *init_tty_object(int id) {
    tty_object_t *tty_obj = malloc(sizeof(tty_object_t));
    bzero(tty_obj->buf, MAX_BUFFER_LEN);
    tty_obj->id = id;
    tty_obj->in_use = false;
    tty_obj->lock = create_lock_any_id();
    tty_obj->num_unconsumed_chars = 0;
    tty_obj->blocked_reads = create_queue();
    tty_obj->blocked_writes = create_queue();
    return tty_obj;
}

tty_object_t *get_tty_object(int id) {
    if (id < NUM_TERMINALS-1) {
        return &tty_objects[id];
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


// //TODO: PIPE FUNCTIONS; keep some and leave others
// bool is_full(int tty_id);

// bool is_empty(int tty_id);

// char read_byte(int tty_id);

// int write_byte(int tty_id);

// int block_pcb_on_pipe(int tty_id, pcb_t* process_block);



