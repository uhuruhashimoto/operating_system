//
// Created by smooth_operator on 10/26/22.
//

#ifndef CURRENT_CHUNGUS_LOAD_PROGRAM
#define CURRENT_CHUNGUS_LOAD_PROGRAM
#include "../data_structures/pcb.h"
#include "../kernel_start.h"
#include "../data_structures/frame_table.h"

/*
 *  Load a program into an existing address space.  The program comes from
 *  the Linux file named "name", and its arguments come from the array at
 *  "args", which is in standard argv format.  The argument "proc" points
 *  to the process or PCB structure for the process into which the program
 *  is to be loaded.
 *
 *  r0 is only true for the init process.
 */

int
LoadProgram(char *name, char *args[], pcb_t* proc);

#endif //CURRENT_CHUNGUS_LOAD_PROGRAM