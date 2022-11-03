#include <yuser.h>

int main(void) {
  Delay(3);
  TracePrintf(1, "EXIT PROCESS -- EXITING\n");
  Exit(33);
  TracePrintf(1, "EXIT PROCESS -- THIS LINE SHOULD NEVER RUN\n");
}