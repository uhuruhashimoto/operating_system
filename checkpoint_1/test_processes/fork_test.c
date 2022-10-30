#include <yuser.h>

int main(void) {
    int pid = Fork();
    TracePrintf(1, "Fork returned %d\n", pid);
    if (pid == 0) {
        TracePrintf(1, "Child back from fork!\n", pid);
        while(1) {
            Pause();
            TracePrintf(1, "Hi! I'm the child and I'll run forever.\n");
        }
    }
    TracePrintf(1, "Parent back from fork!\n", pid);
    while(1) {
        Pause();
        TracePrintf(1, "Hi! I'm the parent and I'll run forever.\n");
    }
}