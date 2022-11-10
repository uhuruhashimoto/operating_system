Yalnix - Current Chungus
Uhuru Hashimoto
Elliot Potter
Oct 27, 2022

This is Yalnix, our operating system for COSC 58 22F. The current kernel boots and handles cloning and switching processes,
as well as handling fork, exec and wait syscalls.

No outstanding bugs (that we know of!)

## <ins> Building </ins>

    `make` or `make all` - builds kernel source and user code
    `make clean` - cleans directory

Note: Makefile paths are set to VBox defaults and may need to be changed. If correct, make commands run as expected.

## <ins> Testing </ins>

### Lock Test
```
./yalnix ./checkpoint_1/test_processes/pipe_lock_cvar_tests/lock_test
```
Creates a process which creates a lock, forks, then acquires the lock. The child process also attempts to acquire the lock,
and is blocked until the parent releases the lock.

#### Lock Destructor Test
```
./yalnix ./checkpoint_1/test_processes/pipe_lock_cvar_tests/lock_destructor_test
```
Creates a process which creates a lock, forks twice, then acquires the lock. The child processes also attempt to acquire
the lock, and are blocked. The parent then destroys the lock and should kill all the children.

### Pipe Test
```
./yalnix ./checkpoint_1/test_processes/pipe_lock_cvar_tests/pipe_test
```
Creates a process which creates a pipe, forks four times and exits. The first two children will both attempt to write to
the pipe. The first will write more bytes (512) than the length of the pipe; the second one will write a few additional bytes.
The last three children will read out these bytes. We expect to see the bytes in the pipes increment in a sane way, with
Child 3 reading 64 ints / 256 bytes, Child 4 reading 64 ints / 256 bytes, and Child 5 reading 2 ints / 8 bytes.

### Cvar Test
```
./yalnix ./checkpoint_1/test_processes/pipe_lock_cvar_tests/cvar_test
```
Tests cvars, using a simplification of the car problem. The process forks, and the child waits on the lock, and then
the cvar.

### Math Test
```
./yalnix ./checkpoint_1/test_processes/math_test
```
Attempts division by 0.