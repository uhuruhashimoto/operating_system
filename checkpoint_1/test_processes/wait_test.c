#include <yuser.h>

int main(const int argc, char **argv) {
  TracePrintf(1, "WAIT_TEST: Forking\n");
  int status;
  int pid = Fork();
  if (pid == 0) {
    TracePrintf(1, "WAIT_TEST: Forked child exec-ing. Should EXIT immediately\n");
    Exec("checkpoint_1/test_processes/exit_test", argv);
    TracePrintf(1, "Exec failed!\n");
    Exit(-2);
  }

  TracePrintf(1, "WAIT_TEST: Waiting for exited process\n");
  // this should return fairly quickly -- wait for 0-1 clock traps
  Wait(&status);
  TracePrintf(1, "WAIT_TEST: Returned from exited process with rc=%d\n", status);

  pid = Fork();
  if (pid == 0) {
    TracePrintf(1, "WAIT_TEST: Forked child exec-ing. Should EXIT after 3 clock ticks\n");
    Exec("checkpoint_1/test_processes/exit_delayed_test", argv);
    TracePrintf(1, "Exec failed!\n");
    Exit(-2);
  }

  TracePrintf(1, "WAIT_TEST: Waiting for exited process\n");
  // this should return fairly slowly -- wait for 3-4 clock traps
  Wait(&status);
  TracePrintf(1, "WAIT_TEST: Returned from exited process with rc=%d\n", status);
}