/*
* Simple program that spams chars to test if exec is working.
*/
#include <yuser.h>
#define PAGESIZE 0x2000

int main(const int argc, char **argv) {
  TracePrintf(1, "Test message program\n");

  for (int i = 0; argv[i] != NULL; i++) {
    TracePrintf(1, "arg = '%s'\n", argv[i]);
  }

    for (int i = 0; i<10; i++) {
        TracePrintf(1, "=======================\n");
    }
    TracePrintf(1, "%s at %p\n", argv, argv);
    for (int i = 0; i<10; i++) {
        TracePrintf(1, "=======================\n");
    }
    Exit(0);
}