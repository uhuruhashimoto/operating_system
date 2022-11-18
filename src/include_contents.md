## ctype.h
Defines a bunch of functions for checking the type of a numerical input (generally characters).

## hardware.h
Defines:
```
pagesize,
page offset,
functions to get a set of pages (?),
virtual mem constants (size, 2 regions, tops and bottoms of both regions),
kernel stack limits and max size,
definition of page table entry,
protection bits used in page table,
user context structure
LinuxContext and KernelContext (opaque to us)
the IDs associated with all our traps
TRAP_MEMORY codes
hardware register names
constants for flushing TLB
terminal stuff -- number, line max size
function definitions for:
    - reading and writing from terminal
    - halting
    - reading and writing registers
    - pausing
    - traceprint
WE WRITE:
    - SetKernelBrk
    - KernelStart
defines KernelContextSwitch
WE WRITE internal thing for switching
debug: ENTER, LEAVE
initial stack frame size
size of null space to allocate after ARGV
```

## yalnix.h
```
defines kernel SYSCALL call numbers for supported kernel calls
pipe buffer len
```

## ylib.h
```
define ERROR, NULL for use in kernel calls
and a bunch of function definition
    - atoi and similar
    - calloc / malloc and similar
    - free
    - heap checker function
    - a bunch of functions for checking types of chars/ints
    - memcopy, memmov, etc
    - string manipulation functions
```

## load_info.h
```
defines how to load memory information from an ELF file (mmap?)
```

## ykernel.h
```
defines a bunch of helper programs
defines KILL error (for use in LoadProgram)
```

## yuser.h
```
Defines a bunch of syscall wrappers
```