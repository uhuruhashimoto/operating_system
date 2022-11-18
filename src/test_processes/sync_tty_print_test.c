/*
* Synchronization testing for tty writes
*/
#include <yuser.h>
#define PAGESIZE 0x2000

int main(const int argc, char **argv) {
  int terminal_num = 0;
  TracePrintf(1, "Testing TTY Write synchronization...\n");
  for (int i=0; i<5; i++) {
    TracePrintf(1, "Forking and writing to terminal %d\n", terminal_num);
    int pid = Fork();
    if (pid == 0) {
      TtyPrintf(terminal_num, "Hello from child %d\n", GetPid());
      Exit(0);
    }
    TtyPrintf(terminal_num, "Hello from parent %d\n", GetPid());
  }
  Exit(0);
}