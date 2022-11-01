#include <yuser.h>
#define PAGESIZE 0x2000

int main(const int argc, char **argv) {
  int pid = Fork();
    if (pid == 0) {
        Exec("exec_test", argv);
        TracePrintf(1, "Exec failed\n");
    }
  // do nothing
  while (1) {

  }
}