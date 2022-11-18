#include <yuser.h>
#define PAGESIZE 0x2000

int main(const int argc, char **argv) {
    for (int i=0; i<argc; i++) {
        TracePrintf(1, "Starting arg %d: %s\n", i, argv[i]);
    }
    Exec("src/test_processes/test_message", argv);
//  char** argv_temp = malloc(sizeof (char* ) * 5);
//  argv_temp[0] = "FOO";
//  argv_temp[1] = "BAR";
//  argv_temp[2] = "BIZ";
//  argv_temp[3] = "BAZ";
//  argv_temp[4] = NULL;
//    // Exec("src/test_processes/test_message", argv_temp);
//    //TracePrintf(1, "Start: Message is %s at addr %p\n", argv, argv);
//    char **msg = (char *[]) {"Hello", "World", NULL};
//    TracePrintf(1, "Start: Message is %s %s at addr %p\n", msg[0], msg[1], msg);
//    Exec("src/test_processes/test_message", msg);
    TracePrintf(1, "Exec failed\n");
}