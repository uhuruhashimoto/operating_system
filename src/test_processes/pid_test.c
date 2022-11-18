#include <yuser.h>
#define PAGESIZE 0x2000

int main(void) {
  int pid = GetPid();
  TracePrintf(1, "My pid is %d\n", pid);

  // do nothing
  while (1) {

  }
}