/* TRAP_MEMORY: Protection violation */
/*
* TESTING COMMENTS 11/13/2022
* This program attempts to write to user text, at which point the kernel detects the violation and kills
* the process (saving its info because it has relatives). This runs successfully. 
*/

#include <yuser.h>

int main(void)
{
  TracePrintf(0,"about to write\n");
    *(int *)0x100000 = 0;
    TracePrintf(0,"wrote!\n");
    Exit(0);
}
