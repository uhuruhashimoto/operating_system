#ifndef CURRENT_CHUNGUS_LOCK_H
#define CURRENT_CHUNGUS_LOCK_H

#include <ykernel.h>
#include "queue.h"

typedef struct lock{
  int lock_id;
  bool locked;
  pcb_t* locking_proc;
  queue_t* blocked_queue;
  struct lock* next_lock;
  struct lock* prev_lock;
} lock_t;

/*
 * Creates a lock with this id
 */
lock_t* create_lock(int lock_id);

/*
 * Creates a lock with any id
 */
lock_t* create_lock_any_id();

/*
 * Finds the lock in the linked list of locks
 */
lock_t* find_lock(int lock_id);

/*
 * Creates a lock with this id
 */
lock_t* create_lock(int lock_id);

/*
 * Attempts to acquire a lock with this id
 * returns ERROR in event of error, SUCCESS otherwise
 */
int acquire(int lock_id);

/*
 * Attempts to release a lock with this id
 * ERROR if lock doesn't exist, this process does not possess lock
 * SUCCESS if successful
 */
int release(int lock_id);

/*
 * Deletes the lock
 */
int delete_lock(lock_t* lock);

#endif //CURRENT_CHUNGUS_LOCK_H