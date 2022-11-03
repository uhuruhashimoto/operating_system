#include <yuser.h>
#define PAGESIZE 0x2000

int main(const int argc, char **argv) {
    //TracePrintf(1, "Start: Message is %s at addr %p\n", argv, argv);
    char **msg = (char *[]) {"Hello", "World", NULL};
    TracePrintf(1, "Start: Message is %s %s at addr %p\n", msg[0], msg[1], msg);
    Exec("checkpoint_1/test_processes/test_message", msg);
    TracePrintf(1, "Exec failed\n");
}