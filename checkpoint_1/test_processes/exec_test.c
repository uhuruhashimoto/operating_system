#include <yuser.h>
#define PAGESIZE 0x2000

int main(const int argc, char **argv) {
  int pid = Fork();
    if (pid == 0) {
        TracePrintf(1, "Child attempting to run pid test...\n");
        Exec("checkpoint_1/test_processes/test_message", argv);
        TracePrintf(1, "Exec failed\n");
    }
  // do nothing
  while (1) {
    TracePrintf(1, "I'm the parent w pid %d, looping forever\n", pid);
    Pause();
  }
}