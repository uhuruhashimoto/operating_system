#include <stdio.h>
#include <yuser.h>
#include <ykernel.h>

/*
 * The iterator process will print itself out once per clock cycle
 */
int main(int argc, char* argv[]) {
  int it = 0;
  while(1) {
    printf("Iterator Process: %d\n", it++);
    Pause();
  }
}