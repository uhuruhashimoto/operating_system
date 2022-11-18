Yalnix - Current Chungus
Uhuru Hashimoto
Elliot Potter
Oct 27, 2022

This is Yalnix, our operating system for COSC 58 22F. The current kernel boots and handles cloning and switching processes,
as well as handling get_pid, delay, and setbrk syscalls. 

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

## Init 
```
./yalnix ./src/test_processes/init
```
In this case, the kernel simply bounces between idle and init process every clock trap. 

### Incrementer Test
```
./yalnix ./src/test_processes/iterator
```
In this test, the kernel alternates between init and idle. 
The idle process will print "DoIdle", while the iterator will print an incrementing value. These two prints alternate.
The program also prints the PID.

### Delay Test
```
./yalnix ./src/test_processes/delay_test
```

### Brk Test
```
./yalnix ./src/test_processes/brk_test
```
Note -- to observe brk_test, stop the OS after about a second of execution.
The idle process will print "DoIdle"; brk_test will malloc and then free some memory, which should trigger an increase
in the brk of 3 pages.

### GetPID Test (in isolation)
```
./yalnix ./src/test_processes/pid_test
```
Because fork is implemented in the next checkpoint, we can't check pid incrementation outside of the kernel. We can,
however, observe the hardware alternation between pids. For our test, we check the user pid of a user process.

