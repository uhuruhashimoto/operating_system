#include <yuser.h>

int main(void) {
  Exit(33);
  TracePrintf(1, "EXIT PROCESS -- THIS LINE SHOULD NEVER RUN\n");
}