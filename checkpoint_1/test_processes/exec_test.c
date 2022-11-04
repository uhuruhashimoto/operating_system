#include <yuser.h>
#define PAGESIZE 0x2000

int main(const int argc, char **argv) {
  char** argv_temp = malloc(sizeof (char* ) * 5);
  argv_temp[0] = "FOO";
  argv_temp[1] = "BAR";
  argv_temp[2] = "BIZ";
  argv_temp[3] = "BAZ";
  argv_temp[4] = NULL;
    Exec("checkpoint_1/test_processes/test_message", argv_temp);
    TracePrintf(1, "Exec failed\n");
}