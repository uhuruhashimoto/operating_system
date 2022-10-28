#include <yuser.h>

int main(void) {
  TracePrintf(1, "Delaying for 3 clocks\n");
  Delay(3);

  TracePrintf(1, "Delaying for 3 clocks\n");
  Delay(3);

  TracePrintf(1, "Delaying for 0 clocks\n");
  Delay(3);

  // do nothing
  while (1) {

  }
}