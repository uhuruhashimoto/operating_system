/*
* This process forks until we run out of physical memory (at child 34). 
*/
#include <yuser.h>

int main(void) {
    while(1){
        int pid = Fork();
        TracePrintf(1, "Fork returned %d\n", pid);
        if (pid == 0) {
            int child_pid = GetPid();
            TracePrintf(1, "Child %d back from fork!\n", child_pid);
            while(1) {
                Pause();
                TracePrintf(1, "Hi! I'm the child with pid %d and I'll run forever.\n", child_pid);
            }
        }
        TracePrintf(1, "Parent %d back from fork!\n", GetPid());
    }
}