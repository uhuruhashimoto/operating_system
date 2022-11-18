#include <yuser.h>

int stack_loop(int id) {
  TracePrintf(1, "STACK_LOOP: id is %d\n", id);
  stack_loop(id+1);
}

/*
 * The iterator process will print itself out during each clock cycle
 */
int main(int argc, char* argv[]) {
  TracePrintf(1, "SEGFAULT_STACK_TEST: Triggering infinite loop to grow the stack!\n");
  stack_loop(0);
}