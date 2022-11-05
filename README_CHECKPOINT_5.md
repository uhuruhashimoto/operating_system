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

### Simply running the OS
```
./yalnix
```
Will halt after attempting to load program "init", unless this is in the top-level directory of the project.
We currently have an "init" executable, but it is located in the /checkpoint_1/test_processes directory.
Options: `-W` to dump core, `-lk [level]` to TracePrint at a level below 1 (for more information).

### Lock Test
```
./yalnix ./checkpoint_1/test_processes/lock_test
```
Creates a process which creates a lock, forks, then acquires the lock. The child process also attempts to acquire the lock,
and is blocked until the parent releases the lock.

### Pipe Test
```
./yalnix ./checkpoint_1/test_processes/lock_test
```
Creates a process which creates a pipe, forks four times and exits. The first two children will both attempt to write to
the pipe. The first will write more bytes than the length of the pipe; the second one will write a few additional bytes.
The second two children will read out these bytes, 50-50. We expect to see the bytes in the pipes increment in a sane way.