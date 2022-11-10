#include <yuser.h>
#include <stdbool.h>

int main(void) {
  TracePrintf(1, "CVAR_DESTRUCTOR_TEST: Testing Cvar functionality\n");
  int cvar_id;
  int rc = CvarInit(&cvar_id);
  if (rc == -1) {
    TracePrintf(1, "CVAR_DESTRUCTOR_TEST: Failed to create a cvar\n");
  }

  int lock_id;
  rc = LockInit(&lock_id);
  if (rc == -1) {
    TracePrintf(1, "CVAR_DESTRUCTOR_TEST: Failed to create a lock\n");
  }

  rc = Fork();
  if (rc == 0) {
    TracePrintf(1, "CVAR_DESTRUCTOR_TEST: Child waking up!\n");
    Acquire(lock_id);

    // normally would put this in a while loop, but doesn't make sense as processes can't edit each other's data
    CvarWait(cvar_id, lock_id);

    TracePrintf(1, "CVAR_DESTRUCTOR_TEST: Child should never get here!\n");

    Exit(0);
  }

  Delay(2);

  // kill the cvar, which should kill the waiting process
  Reclaim(cvar_id);
}