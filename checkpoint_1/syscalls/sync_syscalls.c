// Created by Uhuru Hashimoto on 10/14/22.
// Specs are on Page 32 of Yalnix Manual

#ifndef CURRENT_CHUNGUS_SYNC_SYSCALL_HANDLERS
#define CURRENT_CHUNGUS_SYNC_SYSCALL_HANDLERS
#include <ykernel.h>
#include "../data_structures/lock.h"
#include "../data_structures/cvar.h"
#include "../memory/check_memory.h"
#include "../kernel_start.h"

/*
 * Create a new lock; save its identifier at *lock idp. In case of any error, the value ERROR is returned.
 */
int handle_LockInit(int *lock_idp) {
  TracePrintf(1, "HANDLE_LOCK_INIT: Creating a new lock!\n");

  if (check_memory(lock_idp, sizeof (int)) == ERROR) {
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
  TracePrintf(1, "HANDLE_CVAR_INIT: attempting to create a new cvar\n");
  if (check_memory(cvar_idp, sizeof (int)) == ERROR) {
    return ERROR;
  }

  cvar_t* cvar = create_cvar_any_id();

  // write cvar_idp
  cvar_idp[0] = cvar->id;
  return SUCCESS;
}

/*
 * Signal the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value
ERROR is returned.
 */
int handle_CvarSignal(int cvar_id) {
  TracePrintf(1, "HANDLE_CVAR_SIGNAL: signaling one waiter to wake up\n");
  // signal one of the waiters on the cvar to wake up, and put it in the queue waiting to run
  cvar_t* cvar = find_cvar(cvar_id);
  if (cvar == NULL) {
    TracePrintf(1, "HANDLE_CVAR_SIGNAL: Unable to find a cvar with id %d\n", cvar_id);
    return ERROR;
  }
  pcb_t* next_pcb = remove_from_queue(cvar->blocked_queue);

  // stick the next pcb in the queue
  if (next_pcb != NULL) {
    add_to_queue(ready_queue, next_pcb);
  }
}

/*
 * Broadcast the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the
value ERROR is returned.
 */
int handle_CvarBroadcast(int cvar_id){
  TracePrintf(1, "HANDLE_CVAR_BROADCAST: waking up all waiters\n");
  // wake up all of the waiters, and put them in the queue waiting to run
  cvar_t* cvar = find_cvar(cvar_id);
  if (cvar == NULL) {
    TracePrintf(1, "HANDLE_CVAR_BROADCAST: Unable to find a cvar with id %d\n", cvar_id);
    return ERROR;
  }
  pcb_t* next_pcb = remove_from_queue(cvar->blocked_queue);
  while (next_pcb != NULL) {
    add_to_queue(ready_queue, next_pcb);
    next_pcb = remove_from_queue(cvar->blocked_queue);
  }

  return SUCCESS;
}

/*
 * The kernel-level process releases the lock identified by lock id and waits on the condition variable identified
by cvar id. When the kernel-level process wakes up (e.g., because the condition variable was signaled), it
re-acquires the lock. (Use Mesa-style semantics.)
When the lock is finally acquired, the call returns to userland.
In case of any error, the value ERROR is returned.
 */
int handle_CvarWait(int cvar_id, int lock_id) {
  cvar_t* cvar = find_cvar(cvar_id);
  if (cvar == NULL) {
    TracePrintf(1, "HANDLE_CVAR_WAIT: Unable to find a cvar with id %d\n", cvar_id);
    return ERROR;
  }

  int rc = release(lock_id);
  if (rc == ERROR) {
    TracePrintf(1, "HANDLE_CVAR_WAIT: Unable to release a lock with id %d\n", lock_id);
  }

  // block the running process
  add_to_queue(cvar->blocked_queue, running_process);
  install_next_from_queue(running_process, 1);

  TracePrintf(1, "HANDLE_CVAR_WAIT: Back from block on cvar; now acquiring lock\n");
  return acquire(lock_id);
}

/*
* Kill cvar by cvar id, and any queued children. If necessary,
* we could specify a kill/don't kill option in our input args.
 *
 * kill_children = 0  --> don't kill
 * kill_children = 1  --> do kill
*/
int handle_CvarKill(int cvar_id, int kill_children) {

}

#endif //CURRENT_CHUNGUS_SYNC_SYSCALL_HANDLERS