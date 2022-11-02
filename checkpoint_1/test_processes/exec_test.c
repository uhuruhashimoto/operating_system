#include <yuser.h>
#define PAGESIZE 0x2000

int main(const int argc, char **argv) {
    Exec("checkpoint_1/test_processes/test_message", argv);
    TracePrintf(1, "Exec failed\n");
}