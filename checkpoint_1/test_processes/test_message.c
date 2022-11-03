/*
* Simple program that spams chars to test if exec is working.
*/
#include <yuser.h>
#define PAGESIZE 0x2000

int main(const int argc, char **argv) {
    for (int i = 0; i<10; i++) {
        TracePrintf(1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
    }
    Pause();
}