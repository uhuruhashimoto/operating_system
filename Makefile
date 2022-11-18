#
#	Sample Makefile for Yalnix kernel and user programs.
#	
#	Prepared by Sean Smith and Adam Salem and various Yalnix developers
#	of years past...



# Where's your kernel source?
K_SRC_DIR = /home/cs58/vbs/yalnix_test/current_chungus/src
# K_SRC_DIR = /media/sf_cs58/current_chungus/src

# What are the kernel c and include files?
DATA_STRUCTURES = data_structures/pcb.c data_structures/queue.c data_structures/frame_table.c \
data_structures/pipe.c data_structures/lock.c data_structures/cvar.c data_structures/tty.c

#K_SRCS = $(DATA_STRUCTURES) debug_utils/*.c kernel_start.c kernel_utils.c syscalls/*.c process_management/*.c memory/*.c trap_handlers/*.c
K_SRCS = kernel_start.c kernel_utils.c data_structures/pcb.c data_structures/queue.c data_structures/frame_table.c \
syscalls/io_syscalls.c syscalls/ipc_syscalls.c syscalls/process_syscalls.c \
syscalls/sync_syscalls.c process_management/load_program.c debug_utils/debug.c \
memory/check_memory.c data_structures/pipe.c data_structures/lock.c data_structures/cvar.c \
data_structures/tty.c trap_handlers/trap_handlers.c

K_INCS = $(K_SRCS:%.c=%.h) 

# Where's your user source?
U_SRC_DIR = $(K_SRC_DIR)/test_processes

# What are the user c and include files?
U_SRCS = iterator.c brk_test.c delay_test.c pid_test.c init.c test_message.c exit_test.c exit_delayed_test.c math_test.c \
fork_exec_wait_tests/fork_test.c fork_exec_wait_tests/exec_test.c fork_exec_wait_tests/fork_bomb.c fork_exec_wait_tests/wait_test.c \
fork_exec_wait_tests/pid_increment.c pipe_lock_cvar_tests/lock_test.c pipe_lock_cvar_tests/pipe_test.c pipe_lock_cvar_tests/cvar_test.c \
pipe_lock_cvar_tests/lock_destructor_test.c pipe_lock_cvar_tests/pipe_destructor_test.c pipe_lock_cvar_tests/cvar_destructor_test.c \
tty_tests/tty_print_test.c sync_tty_print_test.c segfault_stack_test.c segfault_random_access_test.c \
class_tests/bigstack.c class_tests/forktest.c class_tests/torture.c class_tests/zero.c mean_memory_tests.c

U_INCS =


#==========================================================
# you should not need to change anything below this line
#==========================================================

#make all will make all the kernel objects and user objects
ALL = $(KERNEL_ALL) $(USER_APPS)
KERNEL_ALL = yalnix


# Automatically generate the list of sources, objects, and includes for the kernel
KERNEL_SRCS = $(K_SRCS:%=$(K_SRC_DIR)/%)
KERNEL_OBJS = $(KERNEL_SRCS:%.c=%.o) 
KERNEL_INCS = $(K_INCS:%=$(K_SRC_DIR)/%) 


# Automatically generate the list of apps, sources, objects, and includes for your userland coden
USER_SRCS = $(U_SRCS:%=$(U_SRC_DIR)/%)
USER_OBJS = $(USER_SRCS:%.c=%.o)
USER_APPS = $(USER_SRCS:%.c=%)
USER_INCS = $(U_INCS:%=$(U_SRC_DIR)/%) 

#write to output program yalnix
YALNIX_OUTPUT = yalnix



#Use the gcc compiler for compiling and linking
CC = gcc

DDIR58 = $(YALNIX_FRAMEWORK)
LIBDIR = $(DDIR58)/lib
INCDIR = $(DDIR58)/include
ETCDIR = $(DDIR58)/etc
#LIBRARY_PATH = $(ETCDIR)/linking/lib

# any extra loading flags...
LD_EXTRA = 

KERNEL_LIBS = $(LIBDIR)/libkernel.a $(LIBDIR)/libhardware.so

# the "kernel.x" argument tells the loader to use the memory layout in the kernel.x file..
KERNEL_LDFLAGS = $(LD_EXTRA) -L$(LIBDIR) -lkernel -lelf  -Wl,-T,$(ETCDIR)/kernel.x  -Wl,-R$(LIBDIR)  -lhardware
LINK_KERNEL = $(LINK.c)

#
#	These definitions affect how your Yalnix user programs are
#	compiled and linked.  Use these flags *only* when linking a
#	Yalnix user program.
#

USER_LIBS = $(LIBDIR)/libuser.a
ASFLAGS = -D__ASM__
CPPFLAGS= -D_FILE_OFFSET_BITS=64 -m32 -fno-builtin -fno-stack-protector -I. -I$(INCDIR) -g -DLINUX


##########################
#Targets for different makes
# all: make all changed components (default)
# clean: remove all output (.o files, temp files, LOG files, TRACE, and yalnix)
# count: count and give info on source files
# list: list all c files and header files in current directory
# kill: close tty windows.  Useful if program crashes without closing tty windows.
# $(KERNEL_ALL): compile and link kernel files
# $(USER_ALL): compile and link user files
# %.o: %.c: rules for setting up dependencies.  Don't use this directly
# %: %.o: rules for setting up dependencies.  Don't use this directly

all: $(ALL)	

clean:
	rm -f *.o *~ TTYLOG* TRACE $(YALNIX_OUTPUT) $(USER_APPS) $(KERNEL_OBJS) $(USER_OBJS) core.* ~/core

count:
	wc $(KERNEL_SRCS) $(USER_SRCS)

list:
	ls -l *.c *.h

kill:
	killall yalnixtty yalnixnet yalnix

no-core:
	rm -f core.*

$(KERNEL_ALL): $(KERNEL_OBJS) $(KERNEL_LIBS) $(KERNEL_INCS)
	$(LINK_KERNEL) -o $@ $(KERNEL_OBJS) $(KERNEL_LDFLAGS)

$(USER_APPS): $(USER_OBJS) $(USER_INCS)
	$(ETCDIR)/yuserbuild.sh $@ $(DDIR58) $@.o










