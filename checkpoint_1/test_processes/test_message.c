/*
* Simple program that spams chars to test if exec is working.
*/
#include <yuser.h>
#define PAGESIZE 0x2000

int main(const int argc, char **argv) {
    for (int i = 0; i<10; i++) {
        TracePrintf(1, "=======================\n");
    }
    for (int i=0; i<argc; i++) {
        TracePrintf(1, "Arg %d: %s\n", i, argv[i]);
    }
    for (int i = 0; i<10; i++) {
        TracePrintf(1, "=======================\n");
    }
    Exit(0);
}