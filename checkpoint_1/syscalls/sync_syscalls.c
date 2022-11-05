// Created by Uhuru Hashimoto on 10/14/22.
// Specs are on Page 32 of Yalnix Manual

#ifndef CURRENT_CHUNGUS_SYNC_SYSCALL_HANDLERS
#define CURRENT_CHUNGUS_SYNC_SYSCALL_HANDLERS
#include <ykernel.h>
#include "../data_structures/lock.h"
#include "../memory/check_memory.h"

/*
 * Create a new lock; save its identifier at *lock idp. In case of any error, the value ERROR is returned.
 */
int handle_LockInit(int *lock_idp) {
  if (check_memory(lock_idp, sizeof int) == ERROR) {
    return ERROR;
  }

  lock_t* new_lock = create_lock_any_id();
  if (new_lock == NULL) {
    TracePrintf(1, "HANDLE_LOCK_INIT: Unable to create a new lock\n");
    return ERROR;
  }

  lock_idp[0] = new_lock->lock_id;

  return SUCCESS;
}

/*
 * Acquire the lock identified by lock id. In case of any error, the value ERROR is returned.
 */
int handle_Acquire(int lock_id) {
  return acquire(lock_id);
}

/*
 * Release the lock identified by lock id. The caller must currently hold this lock. In case of any error, the value
ERROR is returned.
 */
int handle_Release(int lock_id) {
  return release(lock_id);
}

/*
 * Create a new condition variable; save its identifier at *cvar idp. In case of any error, the value ERROR is
returned.
 */
int handle_CvarInit(int *cvar_idp) {
    // create a condition variable, storing its value somewhere in the data structure that will be checked by cvar_wait
}

/*
 * Signal the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value
ERROR is returned.
 */
int handle_CvarSignal(int cvar_id) {
    // signal one of the waiters on the cvar to wake up, and put it in the queue waiting to run
}

/*
 * Broadcast the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the
value ERROR is returned.
 */
int handle_CvarBroadcast(int cvar_id){
    // wake up all of the waiters, and put them in the queue waiting to run
}

/*
 * The kernel-level process releases the lock identified by lock id and waits on the condition variable indentified
by cvar id. When the kernel-level process wakes up (e.g., because the condition variable was signaled), it
re-acquires the lock. (Use Mesa-style semantics.)
When the lock is finally acquired, the call returns to userland.
In case of any error, the value ERROR is returned.
 */
int handle_CvarWait(int cvar_id, int lock_id) {

}



#endif //CURRENT_CHUNGUS_SYNC_SYSCALL_HANDLERS