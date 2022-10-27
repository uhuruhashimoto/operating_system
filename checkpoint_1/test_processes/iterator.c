#include <yuser.h>

/*
 * The iterator process will print itself out during each clock cycle
 */
int main(int argc, char* argv[]) {
  int it = 0;
  while(1) {
    TracePrintf(0, "Iterator Process: %d\n", it++);
    Pause();
  }
}