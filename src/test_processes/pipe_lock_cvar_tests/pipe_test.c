#include <yuser.h>

int main(void) {
  TracePrintf(1, "PIPE_TEST: initializing the pipe\n");
  int pipe_id;
  PipeInit(&pipe_id);
  TracePrintf(1, "PIPE_TEST: finished initializing the pipe\n");

  int rc = Fork();
  if (rc == 0) {
    int num_ints = 512 / (sizeof (int));
    TracePrintf(1, "PIPE_TEST: FIRST CHILD -- PREPARING TO WRITE %d INTS TO THE PIPE\n", num_ints);
    int* buf = malloc(sizeof (int) * num_ints);
    for (int offset = 0; offset < num_ints; offset++) {
      buf[offset] = offset;
    }
    TracePrintf(1, "PIPE_TEST: FIRST CHILD -- WRITING %d INTS TO THE PIPE\n", num_ints);
    PipeWrite(pipe_id, buf, num_ints*sizeof(int));
    Exit(0);
  }

  rc = Fork();
  if (rc == 0) {
    int num_ints = 2;
    TracePrintf(1, "PIPE_TEST: SECOND CHILD -- PREPARING TO WRITE %d INTS TO THE PIPE\n", num_ints);
    int* buf = malloc(sizeof (int) * num_ints);
    for (int offset = 0; offset < num_ints; offset++) {
      buf[offset] = offset + 1000;
    }
    TracePrintf(1, "PIPE_TEST: SECOND CHILD -- WRITING %d INTS TO THE PIPE\n", num_ints);
    PipeWrite(pipe_id, buf, num_ints*sizeof(int));
    Exit(0);
  }

  rc = Fork();
  if (rc == 0) {
    int num_ints = 129;
    TracePrintf(1, "PIPE_TEST: THIRD CHILD -- PREPARING TO READ %d INTS FROM THE PIPE\n", num_ints);
    int* buf = malloc(sizeof (int) * num_ints);
    TracePrintf(1, "PIPE_TEST: THIRD CHILD -- READING %d INTS FROM THE PIPE\n", num_ints);
    PipeRead(pipe_id, buf, num_ints*sizeof(int));

    for (int offset = 0; offset < num_ints; offset++) {
      TracePrintf(1, "THIRD CHILD: %d\n", buf[offset]);
    }
    Exit(0);
  }

  rc = Fork();
  if (rc == 0) {
    int num_ints = 129;
    TracePrintf(1, "PIPE_TEST: FOURTH CHILD -- PREPARING TO READ %d INTS FROM THE PIPE\n", num_ints);
    int* buf = malloc(sizeof (int) * num_ints);
    TracePrintf(1, "PIPE_TEST: FOURTH CHILD -- READING %d INTS FROM THE PIPE\n", num_ints);
    PipeRead(pipe_id, buf, num_ints*sizeof(int));

    for (int offset = 0; offset < num_ints; offset++) {
      TracePrintf(1, "FOURTH CHILD: %d\n", buf[offset]);
    }
    Exit(0);
  }

  rc = Fork();
  if (rc == 0) {
    int num_ints = 129;
    TracePrintf(1, "PIPE_TEST: FIFTH CHILD -- PREPARING TO READ %d INTS FROM THE PIPE\n", num_ints);
    int* buf = malloc(sizeof (int) * num_ints);
    TracePrintf(1, "PIPE_TEST: FIFTH CHILD -- READING %d INTS FROM THE PIPE\n", num_ints);
    PipeRead(pipe_id, buf, num_ints*sizeof(int));

    for (int offset = 0; offset < num_ints; offset++) {
      TracePrintf(1, "FIFTH CHILD: %d\n", buf[offset]);
    }
    Exit(0);
  }

  while (1) {
    Delay(100);
  }
}