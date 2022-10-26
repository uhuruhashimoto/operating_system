#include <ykernel.h>
#include "frame_table.h"

//============================ FRAME TABLE HELPERS ==============================//
/*
return the number of free frames, to help during dynamic allocation. 
This assumes that the kernel is uninterruptable and needs no synchronization (mutexes, etc.)
*/
int get_num_free_frames(char *frame_table, int frame_table_size) {
  int numfree = 0;
  for (int i=0; i<frame_table_size; i++) {
    // sum the number of unused frame table frames (1 is used, 0 unused)
    numfree += (frame_table[i] ? 0 : 1);
  }
  return numfree;
}

/*
Traverses frame table bit vector to find the index
of the next free frame. This assumes that the kernel is uninterruptable
and needs no synchronization (mutexes, etc.)
*/
int get_free_frame(char *frame_table, int frame_table_size) {
  int i = 0;
  while (frame_table[i]) {
    if (i < frame_table_size) {
      i++;
    } 
    else {
      return MEMFULL;
    }
  }
  frame_table[i] = 1;
  return i;
}