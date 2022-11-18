/*
* This program forks off a child and kills it repeatedly, until we reach the maximum allowed PID. 
*/
#include <yuser.h>

int main(void) {
    while(1){
        int pid = Fork();
        if (pid == 0) {
            int child_pid = GetPid();
            TracePrintf(1, "Child %d back from fork!\n", child_pid);
            Exit(0);
        }
        TracePrintf(1, "Parent %d back from fork; waiting for child pid %d!\n", GetPid(), pid);
        int rc = 0;
        Wait(&rc);
    }
}