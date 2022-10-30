#include <yuser.h>

int main(void) {
  int pid = Fork();
  TracePrintf(1, "USERLAND: Back from fork\n");
  if (pid == 0) {
    TracePrintf(1, "Child back from fork! I will now loop forever\n", pid);
    while(1) {}
  }
  TracePrintf(1, "Parent back from fork!\n", pid);
  while(1) {}
}