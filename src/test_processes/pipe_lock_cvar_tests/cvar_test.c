#include <yuser.h>
#include <stdbool.h>

int main(void) {
  TracePrintf(1, "CVAR_TEST: Testing Cvar functionality\n");
  int cvar_id;
  int rc = CvarInit(&cvar_id);
  if (rc == -1) {
    TracePrintf(1, "CVAR_TEST: Failed to create a cvar\n");
  }

  int lock_id;
  rc = LockInit(&lock_id);
  if (rc == -1) {
    TracePrintf(1, "CVAR_TEST: Failed to create a lock\n");
  }

  // False --- left, True -- right
  bool cars_on_road = false;
  bool road_direction = false;

  rc = Fork();
  if (rc == 0) {
    TracePrintf(1, "CVAR_TEST: Child waking up!\n");
    Acquire(lock_id);

    // normally would put this in a while loop, but doesn't make sense as processes can't edit each other's data
    CvarWait(cvar_id, lock_id);

    TracePrintf(1, "CVAR_TEST: Child now has lock and is switching road direction!\n");

    cars_on_road = true;
    road_direction = false;
    Release(lock_id);

    Delay(2);
    cars_on_road = false;

    TracePrintf(1, "CVAR_TEST: Child is exiting\n");
    Exit(0);
  }

  // acquire the lock
  Acquire(lock_id);
  cars_on_road = true;
  road_direction = true;
  Release(lock_id);

  Delay(2);
  cars_on_road = false;
  // signal the cvar to wake up child
  CvarSignal(cvar_id);
}