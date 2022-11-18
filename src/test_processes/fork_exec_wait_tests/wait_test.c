#include <yuser.h>

int main(const int argc, char **argv) {
  // THE FIRST TEST -- WAIT FOR A PROCESS THAT EXITS IMMEDIATELY
  TracePrintf(1, "WAIT_TEST: TEST 1\n");
  TracePrintf(1, "WAIT_TEST: Forking\n");
  int status;
  int pid = Fork();
  if (pid == 0) {
    TracePrintf(1, "WAIT_TEST: Forked child exec-ing. Should EXIT immediately\n");
    Exec("src/test_processes/exit_test", argv);
    TracePrintf(1, "Exec failed!\n");
    Exit(-2);
  }

  TracePrintf(1, "WAIT_TEST: Waiting for exited process\n");
  // this should return fairly quickly -- wait for 0-1 clock traps
  Wait(&status);
  TracePrintf(1, "WAIT_TEST: Returned from exited process with rc=%d (should be 33)\n", status);

  // THE SECOND TEST -- WAIT FOR A PROCESS THAT EXITS AFTER 3 TICKS
  TracePrintf(1, "WAIT_TEST: TEST 2\n");
  pid = Fork();
  if (pid == 0) {
    TracePrintf(1, "WAIT_TEST: Forked child exec-ing. Should EXIT after 3 clock ticks\n");
    Exec("src/test_processes/exit_delayed_test", argv);
    TracePrintf(1, "Exec failed!\n");
    Exit(-2);
  }

  TracePrintf(1, "WAIT_TEST: Waiting for exited process\n");
  // this should return fairly slowly -- wait for 3-4 clock traps
  Wait(&status);
  TracePrintf(1, "WAIT_TEST: Returned from exited process with rc=%d (should be 44)\n", status);

  // THE THIRD TEST -- WAIT FOR A PROCESS THAT HAS ALREADY EXITED
  // this should take precedence over a process that forked earlier but is delayed
  TracePrintf(1, "WAIT_TEST: TEST 3\n");
  pid = Fork();
  if (pid == 0) {
    TracePrintf(1, "WAIT_TEST: Forked child exec-ing. Should EXIT after 3 clock ticks\n");
    Exec("src/test_processes/exit_delayed_test", argv);
    TracePrintf(1, "Exec failed!\n");
    Exit(-2);
  }

  pid = Fork();
  if (pid == 0) {
    TracePrintf(1, "WAIT_TEST: Forked child exec-ing. Should EXIT immediately\n");
    Exec("src/test_processes/exit_test", argv);
    TracePrintf(1, "Exec failed!\n");
    Exit(-2);
  }

  Delay(1);

  TracePrintf(1, "WAIT_TEST: Waiting for exited process\n");
  // this should return fairly quickly -- wait for 0-1 clock traps
  Wait(&status);
  TracePrintf(1, "WAIT_TEST: Returned from exited process with rc=%d (should be 33)\n", status);
}