#include <yuser.h>

int main(void) {
  int error = 1 / 0;
  TracePrintf(1, "Uh oh, we passed a division by 0! %d\n", error);
}