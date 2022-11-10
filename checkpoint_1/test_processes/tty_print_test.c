/*
* Simple program that prints to the terminal.
*/
#include <yuser.h>
#define PAGESIZE 0x2000

int main(const int argc, char **argv) {
  TracePrintf(1, "1: Test message program\n");

  TtyPrintf(0, "Hello terminal 0 from userland\n");
  TtyPrintf(1, "Hello terminal 1 from userland\n");
  TtyPrintf(2, "Hello terminal 2 from userland\n");
  TtyPrintf(3, "Hello terminal 3 from userland\n");

  Exit(0);
}