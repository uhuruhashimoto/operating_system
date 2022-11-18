#include <ykernel.h>
#include <hardware.h>
#include "../kernel_start.h"
#include "../kernel_utils.h"
#include "trap_handlers.h"
#include "../data_structures/frame_table.h"
#include "../syscalls/io_syscalls.h"
#include "../syscalls/ipc_syscalls.h"
#include "../syscalls/process_syscalls.h"
#include "../syscalls/sync_syscalls.h"
#include "../data_structures/queue.h"
#include "../debug_utils/debug.h"
#include "../data_structures/tty.h"

// the number of pages away from the user stack we can be and still allow the stack to expand
int PAGES_AWAY_FROM_USER_STACK = 2;

/*
 * Handle traps to the kernel
 */
void handle_trap_kernel(UserContext* context) {
  int rc = 0;
  int trap_type = context->code;
  TracePrintf(1, "Handling kernel trap with code %x\n", trap_type);

  // switch based on the trap type
  switch (trap_type) {
    // process syscalls
    case YALNIX_FORK:
      rc = handle_Fork();
      break;
    case YALNIX_EXEC:
      memcpy(running_process->uctxt, context, sizeof(UserContext));
      rc = handle_Exec((char *)context->regs[0], (char **) context->regs[1]);
      memcpy(context, running_process->uctxt, sizeof(UserContext));
      break;
    case YALNIX_EXIT:
      handle_Exit(context->regs[0]);
      break;
    case YALNIX_WAIT:
      rc= handle_Wait((int *)context->regs[0]); //TODO cast args as temporary solution
      break;
    case YALNIX_GETPID:
      rc = handle_GetPid();
      break;
    case YALNIX_BRK:
      rc = handle_Brk((void *)context->regs[0]); //TODO cast args as temporary solution
      break;
    case YALNIX_DELAY:
      rc = handle_Delay(context->regs[0]);
      break;

    // TTY Syscalls
    case YALNIX_TTY_READ:
      handle_TtyRead(context->regs[0], (void *)context->regs[1], context->regs[2]);
      break;
    case YALNIX_TTY_WRITE:
      handle_TtyWrite(context->regs[0], (void *)context->regs[1], context->regs[2]);
      break;

    // TODO -- what are YALNIX_REGISTER etc?
    // TODO -- what are YALNIX_READ_SECTOR etc?

    // IPC
    case YALNIX_PIPE_INIT:
      rc = handle_PipeInit((int *)context->regs[0]);
      break;
    case YALNIX_PIPE_READ:
      rc = handle_PipeRead(context->regs[0], (void *)context->regs[1], context->regs[2]);
      break;
    case YALNIX_PIPE_WRITE:
      rc = handle_PipeWrite(context->regs[0], (void *)context->regs[1], context->regs[2]);
      break;

    // NOP
    case YALNIX_NOP:
      // do nothing!
      break;

    // TODO -- semaphore stuff?

    case YALNIX_LOCK_INIT:
      rc = handle_LockInit((int *)context->regs[0]);
      break;
    case YALNIX_LOCK_ACQUIRE:
      rc = handle_Acquire(context->regs[0]);
      break;
    case YALNIX_LOCK_RELEASE:
      rc = handle_Release(context->regs[0]);
      break;
    case YALNIX_CVAR_INIT:
      rc = handle_CvarInit((int *)context->regs[0]);
      break;
    case YALNIX_CVAR_SIGNAL:
      rc = handle_CvarSignal(context->regs[0]);
      break;
    case YALNIX_CVAR_BROADCAST:
      rc = handle_CvarBroadcast(context->regs[0]);
      break;
    case YALNIX_CVAR_WAIT:
      rc = handle_CvarWait(context->regs[0], context->regs[1]);
      break;
    // use the ranges for each type of object id to figure out what type of object we're killing
    case YALNIX_RECLAIM:
      int id = context->regs[0];

      // each of these processes default to killing all waiting children.
      if (id >= min_possible_pipe_id && id <= max_possible_pipe_id) {
        rc = handle_PipeKill(id, 1);
      }
      else if (id >= min_possible_lock_id && id <= max_possible_lock_id) {
        rc = handle_LockKill(id, 1);
      }
      else if (id >= min_possible_cvar_id && id <= max_possible_cvar_id) {
        rc = handle_CvarKill(id, 1);
      }
      break;

    // TODO -- YALNIX_ABORT
    // TODO -- YALNIX_BOOT
  }
  context->regs[0] = rc;

}

/*
 * Handle traps to clock -- starts the next process in the ready queue
 */
void handle_trap_clock(UserContext* context) {
  TracePrintf(1, "TRAP_CLOCK: Our kernel hit the clock trap\n");

  if (ready_queue == NULL) {
    TracePrintf(1, "TRAP_CLOCK/DELAY: NULL READY QUEUE\n");
    return;
  }

  // handle Delay:
  if (delayed_processes != NULL) {
    if (delayed_processes->prev_pcb == NULL) {
      TracePrintf(3, "TRAP_CLOCK/DELAY: NULL prev_pcb\n");
    }

    if (delayed_processes->next_pcb == NULL) {
      TracePrintf(3, "TRAP_CLOCK/DELAY: NULL next_pcb\n");
    } else {
      TracePrintf(3, "TRAP_CLOCK/DELAY: Going from %d to %d\n", delayed_processes, delayed_processes->next_pcb);
    }

    // go through all processes in the delay data structure
    pcb_t* next_process = delayed_processes;
    pcb_t *stored_process = NULL;
    TracePrintf(1, "TRAP_CLOCK/DELAY: Remaining Ticks: %d\n\n", next_process->delayed_clock_cycles);
    while (next_process != NULL) {
      // Because add_to_queue adjust the processes next and previous pointers, we need to save them
      stored_process = next_process->next_pcb;
      // decrement their delays
      next_process->delayed_clock_cycles--;
      TracePrintf(1, "TRAP_CLOCK/DELAY: Delayed process with id %d now has %d clock cycles remaining\n",
                  next_process->pid, next_process->delayed_clock_cycles);

      // if any process gets a delay of 0 or less, put it back into the ready queue
      if (next_process->delayed_clock_cycles <= 0) {
        TracePrintf(1, "Delayed process with id %d will be put in the ready queue\n", next_process->pid);

        // remove it from the delay data structure
        if (next_process->prev_pcb == NULL) {
         TracePrintf(1, "PrevPCB is NULL\n");
          // next_pcb may be NULL, this is OK
          if (next_process->next_pcb != NULL) {
            delayed_processes = next_process->next_pcb;
            delayed_processes->prev_pcb = NULL;
          }
          else {
            add_to_queue(ready_queue, next_process);
            delayed_processes = NULL;
            break;
          }
        }
        else {
        //  TracePrintf(1, "PrevPCB is NOT NULL\n");
          // muck with pointers to remove our process from the queue
          next_process->prev_pcb->next_pcb = next_process->next_pcb;
          if (next_process->next_pcb != NULL) {
            next_process->next_pcb->prev_pcb = next_process->prev_pcb;
          }
        }
        add_to_queue(ready_queue, next_process);
      }

//      TracePrintf(1, "Going from %d to %d\n", next_process, next_process->next_pcb);
      next_process = stored_process;
    }
  }

  TracePrintf(3, "TRAP_CLOCK/DELAY: LEFT THE WHILE LOOP, %d\n", delayed_processes);
  pcb_t* old_process = running_process;

  // get the next process from the queue
  install_next_from_queue(old_process, 0);
}

/*
 * Handles all other traps
 */
void handle_trap_unhandled(UserContext* context) {
  TracePrintf(1, "This trap is not yet implemented\n");
}

/***************** FUTURE HANDLERS *********************/
/*
 * These will be implemented after Checkpoint 1... but we're still writing pseudocode for it!
 */


/*
 * Abort the current user process
 */
void handle_trap_illegal(UserContext* context) {
  TracePrintf(1, "TRAP_ILLEGAL: Killing the user process\n");
  // abort the current process
  delete_process(running_process, ERROR, true);
}

/*
 * Enlarges the user memory if it's an implicit request for more memory
 * otherwise kills the process
 */
void handle_trap_memory(UserContext* context) {
  TracePrintf(1, "TRAP_MEMORY: Attempting to handle a segfault in user space!\n");

  // what is the lowest page in the stack?
  int stack_page_id = (UP_TO_PAGE(VMEM_1_SIZE) >> PAGESHIFT) - 1;
  // keep running down the page table until we hit an invalid page
  while (running_process->region_1_page_table[stack_page_id].valid) {
    stack_page_id--;
  }
  // set the stack page id to be the last valid page
  stack_page_id++;

  // check if the address being touched is PAGES_AWAY_FROM_USER_STACK pages or less away from the top of the stack
  int address = (int)(context->addr);
  int page = (address - UP_TO_PAGE(VMEM_1_SIZE)) >> PAGESHIFT;

  TracePrintf(1, "%d, %d\n", stack_page_id, page);

  // make sure we're close to the stack and not close to the heap, and that we're not above user space
  if (stack_page_id <= page + PAGES_AWAY_FROM_USER_STACK && stack_page_id > page &&
  !running_process->region_1_page_table[page-1].valid
  ) {
    TracePrintf(1, "TRAP_MEMORY: Close enough to the stack that we're giving you benefit of the doubt...\n");

    // allocates new stack pages
    int iteration_start = 0;
    while (page < stack_page_id) {
      iteration_start = get_free_frame(frame_table_global->frame_table, frame_table_global->frame_table_size, iteration_start);

      if (iteration_start == MEMFULL) {
        TracePrintf(1, "TRAP_MEMORY: No free frames to handle segfault!\n");
        // deletes the process
        delete_process(running_process, -1, true);
      }

      TracePrintf(1, "TRAP_MEMORY: Found a free frame to handle segfault!\n");
      running_process->region_1_page_table[page].valid = 1;
      running_process->region_1_page_table[page].prot = (PROT_READ | PROT_WRITE);
      running_process->region_1_page_table[page].pfn = iteration_start;

      page++;
    }


  }
  else {
    TracePrintf(1, "TRAP_MEMORY: Somewhere you shouldn't be, buddy. Die!\n");
    print_reg_1_page_table(running_process, 0, "");

    // Halt();
    // deletes the process
    delete_process(running_process, -1, true);
  }
}

/*
 * Aborts current user process
 */
void handle_trap_math(UserContext* context) {
  TracePrintf(1, "TRAP_MATH: Aborting the user process\n");
  // abort the user process
  // run the next process on the ready queue
  delete_process(running_process, ERROR, true);
}

/*
 * Hardware detected a new line in the terminal
 */
void handle_trap_tty_receive(UserContext* context) {
  TracePrintf(1, "HIT TTY RECEIVE TRAP\n");
  int tty_id = context->code;
  tty_object_t* tty = get_tty_object(tty_id);
  if (tty == NULL) {
    TracePrintf(1, "TRAP_TTY_RECEIVE: There is no tty with id %d\n", tty_id);
    return;
  }

  // read input from terminal with TtyReceive
  int line_length = TtyReceive(tty_id, &tty_buffer, TERMINAL_MAX_LINE);
  TracePrintf(1, "TRAP_TTY_RECEIVE RESULT: tty_id: %d, tty_buffer: %s\n", tty_id, tty_buffer);

  // save into a terminal buffer
  for (int i = 0; i < line_length; i++) {
    // stop writing if we hit ERROR (run out of space on the buffer)
    if (tty_buf_write_byte(tty, tty_buffer[i]) == ERROR) {
      TracePrintf(1, "HANDLE_TTY_RECEIVE: Rotary queue has no more space to store terminal input\n");
      // TODO -- do we want to somehow swap this process out?
      break;
    }
  }

  handle_CvarBroadcast(tty->read_cvar->id);
}

/*
 * A line being written to the terminal has completed
 */
void handle_trap_tty_transmit(UserContext* context) {
  int tty_id = context->code;
  tty_object_t* tty = get_tty_object(tty_id);
  if (tty == NULL) {
    TracePrintf(1, "There is no tty with id %d\n", tty_id);
    return;
  }
  TracePrintf(1, "TRAP_TTY_TRANSMIT: tty_id = %d\n", tty_id);
  // wake up the waiting process
  add_to_queue(ready_queue, tty->writing_proc);
}

