#include <ykernel.h>
#include "queue.h"
#include "tty.h"

tty_object_t *init_tty_object(int id) {
    tty_object_t *tty_obj = malloc(sizeof(tty_object_t));
    bzero(tty_obj->buf, MAX_BUFFER_LEN);
    tty_obj->id = id;
    tty_obj->blocked_reads = create_queue();
    tty_obj->blocked_writes = create_queue();
    return tty_obj;
}


// //TODO: PIPE FUNCTIONS; keep some and leave others
// bool is_full(int tty_id);

// bool is_empty(int tty_id);

// char read_byte(int tty_id);

// int write_byte(int tty_id);

// int block_pcb_on_pipe(int tty_id, pcb_t* process_block);

// /********** unblock_pcb_on_pipe *************/
// /*
//  * Gets the pcb_t* at head of pipe, places it in ready queue, returns a pointer to it
//  */
// pcb_t* unblock_pcb_on_pipe(int tty_id);

