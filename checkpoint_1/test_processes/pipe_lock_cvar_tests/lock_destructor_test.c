#include <yuser.h>

int main(void) {
  TracePrintf(1, "LOCK_TEST: Initializing the lock\n");

  int lock_id;
  LockInit(&lock_id);
  TracePrintf(1, "LOCK_TEST: Finished initializing the lock\n");

  int rc = Fork();
  if (rc == 0) {
    TracePrintf(1, "LOCK_TEST: Child 1 attempting to acquire the lock\n");
    Acquire(lock_id);
    TracePrintf(1, "LOCK_TEST: Child 1 should never get here!\n");

    Exit(0);
  }

  rc = Fork();
  if (rc == 0) {
    TracePrintf(1, "LOCK_TEST: Child 2 attempting to acquire the lock\n");
    Acquire(lock_id);
    TracePrintf(1, "LOCK_TEST: Child 2 should never get here\n");

    Exit(0);
  }

  TracePrintf(1, "LOCK_TEST: Parent acquiring the lock\n");
  Acquire(lock_id);
  TracePrintf(1, "LOCK_TEST: Parent finished acquiring the lock\n");

  Delay(2);

  TracePrintf(1, "LOCK_TEST: Parent destroying the lock\n");
  Reclaim(lock_id);

  // make sure we didn't do anything silly with switching / pcbs
  Wait(&rc);
  TracePrintf(1, "LOCK_TEST: Wait returned with value %d\n", rc);
}