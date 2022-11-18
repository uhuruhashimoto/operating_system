#include <ykernel.h>
#include "queue.h"
#include "pcb.h"



/*
 * Creates a new queue object
 */
queue_t* create_queue() {
  queue_t* new_queue = malloc(sizeof (queue_t));
  if (new_queue == NULL) {
    TracePrintf(1, "CREATE_QUEUE: Failed to allocate memory for a new queue\n");
    return NULL;
  }
  new_queue->size = 0;
  new_queue->head = NULL;
  new_queue->tail = NULL;
}

/*
 * is the queue empty?
 */
bool is_empty(queue_t* queue) {
  if (queue->head == NULL) {
    return true;
  }
  return false;
}

/*
 * Adds the PCB to the back of the ready queue
 */
int add_to_queue(queue_t* queue, pcb_t* pcb)
{
  // add the pcb to the tail of the ready queue
  pcb_t* old_tail = queue->tail;
  queue->tail = pcb;

  TracePrintf(1, "=========================ADDING pcb %d\n", pcb->pid);

  // muck with pointers so that the previous tail points to this tail
  pcb->prev_pcb = old_tail;
  if (old_tail != NULL) {
    old_tail->next_pcb = pcb;
  }
  // there were no elements in the queue
  else {
    queue->head = pcb;
  }

  queue->size++;

  // return 0 on SUCCESS, -1 on ERROR
  return SUCCESS;
}

/*
 * Removes a PCB from the front of the ready queue
 */
pcb_t* remove_from_queue(queue_t* queue)
{
  // get the PCB from the front of the ready queue
  pcb_t* pcb = queue->head;
  if (pcb == NULL) {
    return NULL;
  }
  // move the head of the ready queue
  queue->head = pcb->next_pcb;

  queue->size--;

  // if we have removed the last element from the head, set tail to NULL
  if (queue->head == NULL || queue->size <= 0) {
    queue->tail = NULL;
    queue->head = NULL;
  }

  if (pcb != NULL) {
    TracePrintf(1, "=========================REMOVING pcb %d\n", pcb->pid);
  }

  // return the PCB
  return pcb;
}