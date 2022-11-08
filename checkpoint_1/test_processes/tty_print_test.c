/*
* Simple program that prints to the terminal.
*/
#include <yuser.h>
#define PAGESIZE 0x2000

int main(const int argc, char **argv) {
  TracePrintf(1, "Test message program\n");

  TTyPrintf(1, "Hello from userland\n");
  
  Exit(0);
}