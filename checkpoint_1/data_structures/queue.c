#include "queue.h"
#include "pcb.h"

typedef struct queue {
  pcb_t* head;
  pcb_t* tail;
  int size=0;
} queue_t;

// TODO -- function to create new queue objects

/*
 * Adds the PCB to the back of the ready queue
 */
int add_to_queue(queue_t* queue, pcb_t* pcb)
{
  // add the pcb to the tail of the ready queue

  // muck with pointers so that the previous tail points to this tail

  // return 0 on SUCCESS, -1 on ERROR
}

/*
 * Removes a PCB from the front of the ready queue
 */
pcb_t* remove_from_queue(queue_t* queue)
{
  // get the PCB from the front of the ready queue
  // move the head of the ready queue
  // return the PCB
}

/*
 * Removes a PCB from the front of the queue, and swaps in an idle PCB if queue is empty
 *  For use with the ready queue
 */
pcb_t* SAFE_remove_from_queue(queue_t* queue)
{
  pcb_t* pcb = remove_from_queue(queue);
  if (pcb == NULL) {
    // TODO -- set pcb to be an idle pcb
  }
  return pcb;
}