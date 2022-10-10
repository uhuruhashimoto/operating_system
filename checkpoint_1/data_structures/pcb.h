//
// Created by smooth_operator on 10/8/22.
//

#ifndef CURRENT_CHUNGUS_PCB
#define CURRENT_CHUNGUS_PCB

#include "stdbool.h"
// TODO -- include user context

typedef struct pcb {
  int pid;                                             // the process id
  // uspace: some way to store frames (linked list?)
  // uctxt: user context
  // kctext: kernel context
  // kstack: some way to store frames (linked list?)

  bool hasExited;                                      // whether the process is dead yet
  int rc;                                              // the return code of the process
  pcb_t* next_pcb;                                     // the next pcb in the queue
  pcb_t* prev_pcb;                                     // the previous pcb in the queue
} pcb_t;


#endif //CURRENT_CHUNGUS_PCB