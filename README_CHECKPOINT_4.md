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
We currently have an "init" executable, but it is located in the /src/test_processes directory.
Options: `-W` to dump core, `-lk [level]` to TracePrint at a level below 1 (for more information).

### Fork Test
```
./yalnix ./src/test_processes/fork_exec_wait_tests/fork_test
```
In this test, the init process forks a child, and they both print in an infinite loop, switching back and forth.

### Fork Bomb
```
./yalnix ./src/test_processes/fork_exec_wait_tests/fork_bomb
```
This process will continually fork until Yalnix runs out of memory and halts.

### Exec Test
```
./yalnix ./src/test_processes/fork_exec_wait_tests/exec_test foo bar baz
```
Prints the arguments passed into yalnix on invocation. (foo bar baz)

### Wait Test
```
./yalnix ./src/test_processes/fork_exec_wait_tests/wait_test
```
Tests three things. First, forks and executes a process that will terminate immediately (and waits for it). Second, forks
and executes a process that will terminate after 3 ticks (and waits for it). Third, executes both processes simultaneously.
It waits for one of the two processes to execute.
The expected behavior is (Wait ~1 tick, wait ~3 ticks, wait ~1 tick)
