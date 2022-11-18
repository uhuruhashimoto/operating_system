#ifndef CURRENT_CHUNGUS_FRAME_TABLE_H
#define CURRENT_CHUNGUS_FRAME_TABLE_H

#include <ykernel.h>
#define MEMFULL -1

typedef struct frame_table_struct{
  char *frame_table;
  int frame_table_size;
} frame_table_struct_t;

//=================== FRAME TABLE FUNCTIONS =================//

/*
* return the number of the next free frame, or MEMFULL if memory is insufficient
*/
int get_free_frame(char *frame_table, int frame_table_size, int iteration_start);

/*
* return the number of free frames
*/
int get_num_free_frames(char *frame_table, int frame_table_size);

/*
* Free a frame in the frame table
*/
void free_frame(char *frame_table, int frame_table_size, int frame_num);

#endif //CURRENT_CHUNGUS_FRAME_TABLE_H