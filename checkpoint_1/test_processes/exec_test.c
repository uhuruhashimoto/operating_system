#include <yuser.h>
#define PAGESIZE 0x2000

int main(const int argc, char **argv) {
    for (int i=0; i<argc; i++) {
        TracePrintf(1, "Starting arg %d: %s\n", i, argv[i]);
    }
    Exec("checkpoint_1/test_processes/test_message", argv);
    TracePrintf(1, "Exec failed\n");
}