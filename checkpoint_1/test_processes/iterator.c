#include <yuser.h>

/*
 * The iterator process will print itself out during each clock cycle
 */
int main(int argc, char* argv[]) {
  int it = 0;
  while(1) {
    int pid = GetPid();
    TracePrintf(0, "Pid: %d\n", pid);
    TracePrintf(0, "Iterator Process iteration: %d\n", it++);
    Pause();
  }
}