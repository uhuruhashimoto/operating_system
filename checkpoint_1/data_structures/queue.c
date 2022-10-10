#include "queue.h"
// TODO -- import the ready_queue global

/*
 * Adds the PCB to the back of the ready queue
 */
int add_to_ready_queue(pcb_t* pcb)
{
  // add the pcb to the tail of the ready queue

  // muck with pointers so that the previous tail points to this tail

  // return 0 on SUCCESS, -1 on ERROR
}

/*
 * Removes a PCB from the front of the ready queue
 */
pcb_t* remove_from_ready_queue()
{
  // get the PCB from the front of the ready queue
  // move the head of the ready queue
  // return the PCB
}