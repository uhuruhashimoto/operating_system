#include <yuser.h>

int main(void) {
  TracePrintf(1, "LOCK_TEST: Initializing the lock\n");

  int lock_id;
  LockInit(&lock_id);
  TracePrintf(1, "LOCK_TEST: Finished initializing the lock\n");

  int rc = Fork();
  if (rc == 0) {
    TracePrintf(1, "LOCK_TEST: Child acquiring the lock\n");
    Acquire(lock_id);
    TracePrintf(1, "LOCK_TEST: Child finished acquiring the lock\n");

    Exit(0);
  }

  TracePrintf(1, "LOCK_TEST: Parent acquiring the lock\n");
  Acquire(lock_id);
  TracePrintf(1, "LOCK_TEST: Parent finished acquiring the lock\n");

  Delay(2);

  TracePrintf(1, "LOCK_TEST: Parent releasing the lock\n");
  Release(lock_id);
  TracePrintf(1, "LOCK_TEST: Parent finished releasing the lock\n");
}