#include <ykernel.h>
#include "queue.h"
#include "lock.h"
#include "../kernel_start.h"

/*
 * Finds the lock in the linked list of locks
 */
lock_t* find_lock(int lock_id)
{
  lock_t* next_lock = locks;
  while (next_lock != NULL) {
    if (next_lock->lock_id == lock_id) {
      return next_lock;
    }
    next_lock = next_lock->next_lock;
  }

  return NULL;
}

/*
 * Creates a lock with this id
 */
lock_t* create_lock(int lock_id)
{
  lock_t* new_lock = malloc(sizeof (lock_t));
  if (new_lock == NULL) {
    TracePrintf(1, "CREATE_LOCK: Failed to allocate memory for a new lock\n");
    return NULL;
  }
  new_lock->locking_proc = NULL;
  new_lock->next_lock = NULL;
  new_lock->prev_lock = NULL;
  new_lock->locked = false;

  new_lock->lock_id = lock_id;
  new_lock->blocked_queue = create_queue();

  if (new_lock->blocked_queue == NULL) {
    return NULL;
  }
  return new_lock;
}

/*
 * Creates a lock with any id
 *
 * also puts it into the locks list
 */
lock_t* create_lock_any_id()
{
  int lock_id = ++max_lock_id;
  if (lock_id > max_possible_lock_id) {
    TracePrintf(1, "Run out of ID space to allocate more locks\n");
    return NULL;
  }
  lock_t* new_lock = create_lock(lock_id);
  if (new_lock == NULL) {
    return NULL;
  }

  // stick the lock into the locks list
  lock_t* old_locks = locks;
  locks = new_lock;
  new_lock->next_lock = old_locks;
  if (old_locks != NULL) {
    old_locks->prev_lock = new_lock;
  }

  return new_lock;
}

/*
 * Attempts to acquire a lock with this id
 * returns ERROR in event of error, SUCCESS otherwise
 */
int acquire(int lock_id)
{
  lock_t* lock = find_lock(lock_id);
  if (lock == NULL) {
    TracePrintf(1, "ACQUIRE_LOCK: The lock with id %d does not exist\n", lock_id);
    return ERROR;
  }

  // if the lock is open, claim it
  if (lock->locking_proc == NULL || lock->locking_proc == running_process) {
    TracePrintf(1, "ACQUIRE_LOCK: Process with id %d acquiring open lock with id %d\n", running_process->pid, lock_id);
    lock->locking_proc = running_process;
    return SUCCESS;
  }
  else {
    TracePrintf(1, "ACQUIRE_LOCK: Waiting for lock with id %d\n", lock_id);

    while (lock->locking_proc != NULL && lock->locking_proc != running_process) {
      TracePrintf(1, "ACQUIRE_LOCK: Waiting for lock with id %d\n", lock_id);
      // put the process in the blocked queue for the lock
      add_to_queue(lock->blocked_queue, running_process);
      install_next_from_queue(running_process, 1);
    }

    // now we have the lock
    lock->locking_proc = running_process;
    return SUCCESS;
  }
}

/*
 * Attempts to release a lock with this id
 * ERROR if lock doesn't exist, this process does not possess lock
 * SUCCESS if successful
 */
int release(int lock_id)
{
  TracePrintf(1, "RELEASE_LOCK: Attempting to release the lock with id %d\n", lock_id);

  lock_t* lock = find_lock(lock_id);
  if (lock == NULL) {
    TracePrintf(1, "RELEASE_LOCK: The lock with id %d does not exist\n", lock_id);
    return ERROR;
  }

  if (lock->locking_proc != running_process) {
    TracePrintf(1, "RELEASE_LOCK: The process with id %d doesn't have the lock with id %d\n",
                running_process->pid,
                lock_id);
    return ERROR;
  }

  lock->locking_proc = NULL;
  TracePrintf(1, "RELEASE_LOCK: Passed ifs %d\n", lock_id);
  // if there's another process waiting for the lock, give it the lock
  pcb_t* next_proc = remove_from_queue(lock->blocked_queue);
  TracePrintf(1, "RELEASE_LOCK: Passed queue remove %d\n", lock_id);

  if (next_proc != NULL) {
    lock->locking_proc = next_proc;
    // put this process back into the ready queue
    add_to_queue(ready_queue, next_proc);
  }

  TracePrintf(1, "RELEASE_LOCK: Finishing");
  return SUCCESS;
}

/*
 * Deletes the lock
 */
int delete_lock(lock_t* lock) {
  if (lock != NULL) {
    free(lock->blocked_queue);
  }
  free(lock);
  return SUCCESS;
}