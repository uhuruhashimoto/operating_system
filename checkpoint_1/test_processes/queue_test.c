#include <yuser.h>
#include "../data_structures/pcb.h"
#include "../data_structures/queue.h"

/*
 * The iterator process will print itself out during each clock cycle
 */
int main(int argc, char* argv[]) {
  queue_t* queue = create_queue();
  int it = 0;
  while(1) {
    while(it < 10) {
      TracePrintf(0, "Iterator Process: %d\n", it++);
      Pause();
    }

    pcb_t* next_pcb;
    while ((next_pcb = ) != NULL) {

    }

    it = 0;
  }
}