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

  if (check_memory(lock_idp, sizeof (int), false, true, false, false) == ERROR) {
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
* Kill lock by lock id, and any queued children waiting for lock. If necessary,
* we could specify a kill/don't kill option in our input args.
 *
 * kill_children = 0  --> don't kill
 * kill_children = 1  --> do kill
*/
int handle_LockKill(int lock_id, int kill_children) {
  TracePrintf(1, "HANDLE_LOCK_KILL: Attempting to delete a lock with id %d\n", lock_id);

  lock_t* found_lock = find_lock(lock_id);
  if (found_lock == NULL) {
    TracePrintf(1, "HANDLE_LOCK_KILL: Unable to find a lock with id %d\n", lock_id);
    return ERROR;
  }

  // kill children, or put them in ready queue
  // clear the read-blocked children
  pcb_t* next_child = remove_from_queue(found_lock->blocked_queue);
  while (next_child != NULL) {
    if (kill_children == 1) {
      delete_process(next_child, ERROR, false);
    }
    else {
      add_to_queue(ready_queue, next_child);
    }
    next_child = remove_from_queue(found_lock->blocked_queue);
  }

  // stitch the lock list together
  if (found_lock == locks) {
    locks = found_lock->next_lock;
  }
  if (found_lock->prev_lock != NULL) {
    found_lock->prev_lock->next_lock = found_lock->next_lock;
  }
  if (found_lock->next_lock != NULL) {
    found_lock->next_lock->prev_lock = found_lock->prev_lock;
  }

  // delete the lock
  delete_lock(found_lock);

  return SUCCESS;
}

/*
 * Create a new condition variable; save its identifier at *cvar idp. In case of any error, the value ERROR is
returned.
 */
int handle_CvarInit(int *cvar_idp) {
  TracePrintf(1, "HANDLE_CVAR_INIT: attempting to create a new cvar\n");
  if (check_memory(cvar_idp, sizeof (int), false, true, false, false) == ERROR) {
    return ERROR;
  }

  cvar_t* cvar = create_cvar_any_id();
  if (cvar == NULL) {
    TracePrintf(1, "HANDLE_CVAR_INIT: Unable to create a new cvar\n");
    return ERROR;
  }

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
  TracePrintf(1, "HANDLE_CVAR_WAIT: Blocking on cvar with id %d\n", cvar_id);

  cvar_t* cvar = find_cvar(cvar_id);
  if (cvar == NULL) {
    TracePrintf(1, "HANDLE_CVAR_WAIT: Unable to find a cvar with id %d\n", cvar_id);
    return ERROR;
  }
  TracePrintf(1, "HANDLE_CVAR_WAIT: found cvar\n");

  int rc = release(lock_id);
  // NOTE: does NOT exit in failure case
  if (rc == ERROR) {
    TracePrintf(1, "HANDLE_CVAR_WAIT: Unable to release a lock with id %d\n", lock_id);
  }
  TracePrintf(1, "HANDLE_CVAR_WAIT: Passed lock release\n");

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
  TracePrintf(1, "HANDLE_CVAR_KILL: Attempting to delete a cvar with id %d\n", cvar_id);

  cvar_t* found_cvar = find_cvar(cvar_id);
  if (found_cvar == NULL) {
    TracePrintf(1, "HANDLE_CVAR_KILL: Unable to find a cvar with id %d\n", cvar_id);
    return ERROR;
  }

  // kill children, or put them in ready queue
  // clear the blocked children
  pcb_t* next_child = remove_from_queue(found_cvar->blocked_queue);
  while (next_child != NULL) {
    if (kill_children == 1) {
      delete_process(next_child, ERROR, false);
    }
    else {
      add_to_queue(ready_queue, next_child);
    }
    next_child = remove_from_queue(found_cvar->blocked_queue);
  }

  // stitch the cvar list together
  if (found_cvar == cvars) {
    cvars = found_cvar->next_cvar;
  }
  if (found_cvar->prev_cvar != NULL) {
    found_cvar->prev_cvar->next_cvar = found_cvar->next_cvar;
  }
  if (found_cvar->next_cvar != NULL) {
    found_cvar->next_cvar->prev_cvar = found_cvar->prev_cvar;
  }

  // delete the cvar
  delete_cvar(found_cvar);

  return SUCCESS;
}

#endif //CURRENT_CHUNGUS_SYNC_SYSCALL_HANDLERS