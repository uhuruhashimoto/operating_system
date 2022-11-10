#include <yuser.h>

#include <yuser.h>

int main(void) {
  TracePrintf(1, "PIPE_TEST: Initializing the pipe\n");

  int pipe_id;
  PipeInit(&pipe_id);
  TracePrintf(1, "PIPE_TEST: Finished initializing the pipe\n");

  int rc = Fork();
  if (rc == 0) {
    TracePrintf(1, "PIPE_TEST: Child 1 attempting to read the pipe\n");
    int* buf = malloc(sizeof (int) * 5);
    PipeRead(pipe_id, buf, 5);
    TracePrintf(1, "PIPE_TEST: Child 1 should never get here!\n");

    Exit(0);
  }

  rc = Fork();
  if (rc == 0) {
    TracePrintf(1, "PIPE_TEST: Child 2 attempting to read the pipe\n");
    int* buf = malloc(sizeof (int) * 5);
    PipeRead(pipe_id, buf, 5);
    TracePrintf(1, "PIPE_TEST: Child 2 should never get here\n");

    Exit(0);
  }

  Delay(2);

  TracePrintf(1, "PIPE_TEST: Parent destroying the pipe\n");
  Reclaim(pipe_id);

  // make sure we didn't do anything silly with switching / pcbs
  Wait(&rc);
  TracePrintf(1, "PIPE_TEST: Wait returned with value %d\n", rc);
}