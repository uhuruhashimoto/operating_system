//
// Created by smooth_operator on 10/8/22.
// Specs are on Page 29 of Yalnix Manual
//

#ifndef CURRENT_CHUNGUS_IPC_SYSCALL_HANDLERS
#define CURRENT_CHUNGUS_IPC_SYSCALL_HANDLERS

/*
 * Create a new pipe; save its identifier at *pipe_idp. (See the header files for the length of the pipe’s internal
buffer.) In case of any error, the value ERROR is returned.
 */
int handle_PipeInit(int *pipe_idp);

/*
 * Read len consecutive bytes from the named pipe into the buffer starting at address buf, following the standard
semantics:
– If the pipe is empty, then block the caller.
– If the pipe has plen ≤ len unread bytes, give all of them to the caller and return.
– If the pipe has plen > len unread bytes, give the first len bytes to caller and return. Retain the unread
plen − len bytes in the pipe.
In case of any error, the value ERROR is returned. Otherwise, the return value is the number of bytes read.
 */
int handle_PipeRead(int pipe_id, void *buf, int len);

/*
 * Write the len bytes starting at buf to the named pipe. (As the pipe is a FIFO buffer, these bytes should be
appended to the sequence of unread bytes currently in the pipe.) Return as soon as you get the bytes into the
buffer. In case of any error, the value ERROR is returned. Otherwise, return the number of bytes written.
 */
int handle_PipeWrite(int pipe_id, void *buf, int len);

/*
* Kill pipe by pipe_id, and any queued children waiting for pipe input. If necessary, 
* we could specify a kill/don't kill option in our input args.
*/
int handle_PipeKill(int pipe_id, int kill_children);

#endif //CURRENT_CHUNGUS_IPC_SYSCALL_HANDLERS