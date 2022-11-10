//
// Created by smooth_operator on 10/8/22.
// Specs are on Page 32 of Yalnix Manual
//

#ifndef CURRENT_CHUNGUS_SYNC_SYSCALL_HANDLERS
#define CURRENT_CHUNGUS_SYNC_SYSCALL_HANDLERS
#include <ykernel.h>

/*
 * Create a new lock; save its identifier at *lock idp. In case of any error, the value ERROR is returned.
 */
int handle_LockInit(int *lock_idp);

/*
 * Acquire the lock identified by lock id. In case of any error, the value ERROR is returned.
 */
int handle_Acquire(int lock_id);

/*
 * Release the lock identified by lock id. The caller must currently hold this lock. In case of any error, the value
ERROR is returned.
 */
int handle_Release(int lock_id);

/*
 * Kill a lock with this id
 */
int handle_LockKill(int lock_id, int kill_children);

/*
 * Create a new condition variable; save its identifier at *cvar idp. In case of any error, the value ERROR is
returned.
 */
int handle_CvarInit(int *cvar_idp);

/*
 * Signal the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value
ERROR is returned.
 */
int handle_CvarSignal(int cvar_id);

/*
 * Broadcast the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the
value ERROR is returned.
 */
int handle_CvarBroadcast(int cvar_id);

/*
 * The kernel-level process releases the lock identified by lock id and waits on the condition variable indentified
by cvar id. When the kernel-level process wakes up (e.g., because the condition variable was signaled), it
re-acquires the lock. (Use Mesa-style semantics.)
When the lock is finally acquired, the call returns to userland.
In case of any error, the value ERROR is returned.
 */
int handle_CvarWait(int cvar_id, int lock_id);

/*
 * Kill a cvar with this id
 */
int handle_CvarKill(int cvar_id, int kill_children);



#endif //CURRENT_CHUNGUS_SYNC_SYSCALL_HANDLERS