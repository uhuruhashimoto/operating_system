Yalnix - Current Chungus
Uhuru Hashimoto
Elliot Potter
Oct 27, 2022

This is Yalnix, our operating system for COSC 58 22F. The current kernel boots and handles cloning and switching processes,
as well as handling fork, exec and wait syscalls. It also currently supports reading from and writing to terminals (using the TtyPrintf user command).

No outstanding bugs (that we know of!)

## <ins> Building </ins>

    `make` or `make all` - builds kernel source and user code
    `make clean` - cleans directory

Note: Makefile paths are set to VBox defaults and may need to be changed. If correct, make commands run as expected.

## <ins> Testing </ins>

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

### Math Test
```
./yalnix ./src/test_processes/math_test
```
Attempts division by 0.

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

## TTY Print Test
```
./yalnix ./src/test_processes/tty_print_test
```
This test simply prints a message to each terminal, specifying the terminal number so we can see that the messages are received correctly.

## TTY Print Synchronization Test
```
./yalnix ./src/test_processes/sync_tty_print_test
```
This test loops and forks five times, and each time the parent and child each print a message to the system console. By running it multiple times, we can see different interleavings of the messages themselves, but see that they never become truncated or broken up (that is; waiting processes wait their turn to write as expected).

## TTY Read Test
```
./yalnix ./src/test_processes/init
```
Reading from the terminal may be tested by simply running init or idle, and writing characters to individual terminals. Our system puts the messages into a non null-terminated rotary buffer and prints it every time it receives more input. In the future, we will screen this input and may use it to trigger syscalls or run executables (as one would be able to do in a regular shell).
