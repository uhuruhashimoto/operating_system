# Yalnix - Current Chungus
### Uhuru Hashimoto
### Elliot Potter
### November 17, 2022

## <ins> Description </ins>

This is Yalnix, a preemptive, ring-0, monolithic kernel developed for COSC 58 225 (Operating Systems). This kernel is designed to be run on Ubuntu, using configured signal handlers. It currently supports reading to and writing from terminals, basic syscalls, and synchronization primitives. 

Submission Note: We are using both of our late days for an on-time submission by 11:59 pm, November 17.
## <ins> Usage </ins>

`make` or `make all` - builds directory

`make clean` - cleans directory

`./yalnix -W -x [executable]` - boots kernel and enables output terminals

Note: Makefile paths are set to VBox defaults and may need to be changed. If correct, make commands run as expected.

## <ins> Error Handling </ins>

Our kernel seeks to gracefully deal with the following errors: 

Memory Errors
- If we attempt and are unable to allocate memory for a data structure, we generally respond in the following ways:
    - (1) If the data structure is a global structure necessary for basic kernel scheduling, we halt
    - (2) If the data structure is involved in a syscall, we return ERROR to the user

Input Checking
- Our kernel checks both values and memory regions of syscall inputs, and handles incorrect values by 
returning ERROR to the user. 

## <ins> Testing </ins>

Our tests are located in the `test_processes` directory, and split into the following categories. All may be run 
with the `-x` and `-W` flags without a change in performance. 
- Class Tests
    - torture
    - bigstack
    - forktest
    - zero 
- Basic Syscall Tests
    - Fork tests and fork bombs
    - PID incrementation tests (children exit)
    - Exec tests
    - Wait tests
- Synchronization Tests
    - Cvar Tests (including destruction)
    - Pipes (including destruction)
- Terminal Tests
    - Terminal Write Tests 
- Memory Tests
    - Brk test
    - Memory stress tests (mean memory test)
    - Segfault tests
- Miscellaneous Tests/Multiple-Behavior Tests
    - synchronized parent/child write test 
    - PID tests
    - idle and init programs

## Class Tests
### Torture
```
./yalnix -W -x ./src/test_processes/class_tests/torture
```
This test spawns five processes. Two (Bouncer and SonarGal) simply trade pings via a cvar and mutex. Another (Garbage Man) spams random characters to a terminal, and another (Malloc Monster) repeatedly allocates and frees pages. The remaining process (That Annoying Person) prints input in the hopes to break terminal write synchronization. 
This test successfully stress tests our terminal synchronization, process scheduling, and basic syscalls. It does not test our memory constraints, nor our reclamation/garbage collection of resources.

### Forktest
```
./yalnix -W -x ./src/test_processes/class_tests/forktest
```
This test simply forks, and then recurses with both the parent and child (each printing to the terminal).

### Stack
```
./yalnix -W -x ./src/test_processes/class_tests/bigstack
```
This test allocates memory for a large stack, and exits.

### Zero
```
./yalnix -W -x ./src/test_processes/class_tests/zero
```
This test attempts to write to invalid memory, and is killed (never returns).

## Syscall Tests
### Fork Test
```
./yalnix -W -x ./src/test_processes/fork_exec_wait_tests/fork_test
```
This test forks and prints a message from the parent and child indefinitely. 

### Fork Bomb
```
./yalnix -W -x ./src/test_processes/fork_exec_wait_tests/fork_bomb
```
This test forks repeatedly until all physical memory is allocated (at 34 processes).

### Exec Test
```
./yalnix -W -x ./src/test_processes/fork_exec_wait_tests/exec_test
```
This test execs and does not return.

### PID Increment
```
./yalnix -W -x ./src/test_processes/fork_exec_wait_tests/pid_increment
```
This test forks and waits for the child, effectively incrementing the child PID until it reaches the maximum allowed value. 

### Wait Test
```
./yalnix -W -x ./src/test_processes/fork_exec_wait_tests/wait_test
```
This test waits for a process for increasing amounts of time.  

### Delay Test
```
./yalnix -W -x ./src/test_processes/delay_test
```
This test delays for varying amounts of time. 

### Exit Delayed Test
```
./yalnix -W -x ./src/test_processes/exit_delayed_test
```
This test delays and exits. 

### Exit Test
```
./yalnix -W -x ./src/test_processes/exit_test
```
This test exits correctly, without reaching a forbidden print statement.

## Synchronization Tests
### Lock Test
```
./yalnix ./src/test_processes/pipe_lock_cvar_tests/lock_test
```
Creates a process which creates a lock, forks, then acquires the lock. The child process also attempts to acquire the lock,
and is blocked until the parent releases the lock.

#### Lock Destructor Test
```
./yalnix ./src/test_processes/pipe_lock_cvar_tests/lock_destructor_test
```
Creates a process which creates a lock, forks twice, then acquires the lock. The child processes also attempt to acquire
the lock, and are blocked. The parent then destroys the lock and should kill all the children. It waits on the children
to make sure we exterminated them in a safe manner.

### Pipe Test
```
./yalnix ./src/test_processes/pipe_lock_cvar_tests/pipe_test
```
Creates a process which creates a pipe, forks four times and exits. The first two children will both attempt to write to
the pipe. The first will write more bytes (512) than the length of the pipe; the second one will write a few additional bytes.
The last three children will read out these bytes. We expect to see the bytes in the pipes increment in a sane way, with
Child 3 reading 64 ints / 256 bytes, Child 4 reading 64 ints / 256 bytes, and Child 5 reading 2 ints / 8 bytes.

#### Pipe Destructor Test
```
./yalnix ./src/test_processes/pipe_lock_cvar_tests/pipe_destructor_test
```
Creates a process which creates a pipe, forks twice. The child processes attempt to read from the pipe, and are blocked.
The parent then destroys the pipe and should kill all the children. It waits on the children
to make sure we exterminated them in a safe manner.

### Cvar Test
```
./yalnix ./src/test_processes/pipe_lock_cvar_tests/cvar_test
```
Tests cvars, using a simplification of the car problem. The process forks, and the child waits on the lock, and then
the cvar.

### Cvar Destructor Test
```
./yalnix ./src/test_processes/pipe_lock_cvar_tests/cvar_destructor_test
```
The process forks, and the child waits on the lock, and then
the cvar. The parent destroys the cvar, killing the child.

## Terminal Tests
## TTY Print Test
```
./yalnix -W -x ./src/test_processes/tty_tests/tty_print_test
```
This test simply prints a message to each terminal, specifying the terminal number so we can see that the messages are received correctly.

### TTY Print Synchronization Test
```
./yalnix -W -x ./src/test_processes/sync_tty_print_test
```
This test loops and forks five times, and each time the parent and child each print a message to the system console. By running it multiple times, we can see different interleavings of the messages themselves, but see that they never become truncated or broken up (that is; waiting processes wait their turn to write as expected).

### TTY Read Test
```
./yalnix -W -x ./src/test_processes/init
```
Reading from the terminal may be tested by simply running init or idle, and writing characters to individual terminals. Our system puts the messages into a non null-terminated rotary buffer and prints it every time it receives more input. In the future, we will screen this input and may use it to trigger syscalls or run executables (as one would be able to do in a regular shell).

## Memory Tests
### Brk Test
```
./yalnix ./src/test_processes/brk_test
```
This test allocates a large array (requiring changing the brk), reads from it, and frees it. 

### Segfault Stack Test
```
./yalnix ./src/test_processes/segfault_stack_test
```
This test infinitely adds stack frames to the stack. It should continue implicitly adding frames to the stack until the
stack approaches the heap, in which case it terminates the process.

### Segfault Random Access Test
```
./yalnix ./src/test_processes/segfault_random_access_test
```
This test probes 4 different parts of the address space: \0, kernel space, the middle of user space, and above user space

### Mean Memory Test (Black Thumb)
```
./yalnix ./src/test_processes/mean_memory_tests
```
This tests syscalls for robustness with various incorrect inputs. 

## Miscellaneous Tests
### Math Test
```
./yalnix ./src/test_processes/math_test
```
Attempts division by 0.

### PID Test
```
./yalnix ./src/test_processes/pid_test
```
Tests the ability of a process to get its own PID. 

### Init
```
./yalnix ./src/test_processes/init
```
This simply idles. 

### Iterator
```
./yalnix ./src/test_processes/iterator
```
This test idles, incrementing an integer with every clock cycle.

### Test Message
```
./yalnix ./src/test_processes/pid_test
```
This test prints input arguments to test that our load program functionality properly preserves them.
