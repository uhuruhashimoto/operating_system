#include <yuser.h>
#include "../data_structures/pcb.h"
#include "../data_structures/queue.h"

int MAX_IN_QUEUE = 10;

/*
 * The iterator process will print itself out during each clock cycle
 */
int main(int argc, char* argv[]) {
  queue_t* queue = create_queue();
  int it = 0;
  pcb_t* pcb_list = malloc(sizeof (pcb_t) * MAX_IN_QUEUE);

  while(1) {
    while(it < MAX_IN_QUEUE) {
      pcb_t new_pcb;
      new_pcb.pid = it;
      pcb_list[it] = new_pcb;
      add_to_queue(queue, &new_pcb);
      TracePrintf(0, "Loading pcb with id: %d\n", it++);
      Pause();
    }

    pcb_t* next_pcb;
    while ((next_pcb = remove_from_queue(queue)) != NULL) {
      TracePrintf(0, "Removed a pcb with id: %d\n", next_pcb->pid);
      Pause();
    }

    it = 0;
  }
}