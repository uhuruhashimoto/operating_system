#include <yuser.h>
#include <ylib.h>
#include <ykernel.h>

void null_access() {
  TracePrintf(1, "SEGFAULT_RANDOM_ACCESS_TEST: Attempting to read from NULL\n");
  int i = ((int *)(NULL))[0];
  TracePrintf(1, "SEGFAULT_RANDOM_ACCESS_TEST: %d\n", i);
}

void kernel_access() {
  TracePrintf(1, "SEGFAULT_RANDOM_ACCESS_TEST: Attempting to read from address 2000\n");
  int i = ((int *)(2000))[0];
  TracePrintf(1, "SEGFAULT_RANDOM_ACCESS_TEST: %d\n", i);
}

void middle_user_space_access() {
  TracePrintf(1, "SEGFAULT_RANDOM_ACCESS_TEST: Attempting to read from the middle of user space\n");
  int i = ((int *)(UP_TO_PAGE(VMEM_1_BASE + 0.5*VMEM_1_SIZE)))[0];
  TracePrintf(1, "SEGFAULT_RANDOM_ACCESS_TEST: %d\n", i);
}

void above_user_space_access() {
  TracePrintf(1, "SEGFAULT_RANDOM_ACCESS_TEST: Attempting to read from a value above user space\n");
  int i = ((int *)(UP_TO_PAGE(VMEM_1_BASE + 1000*VMEM_1_SIZE)))[0];
  TracePrintf(1, "SEGFAULT_RANDOM_ACCESS_TEST: %d\n", i);
}

int main(int argc, char* argv[]) {
  TracePrintf(1, "SEGFAULT_RANDOM_ACCESS_TEST: Executing tests\n");

  int rc = Fork();
  if (rc == 0) {
    null_access();
  }

  rc = Fork();
  if (rc == 0) {
    kernel_access();
  }

  rc = Fork();
  if (rc == 0) {
    middle_user_space_access();
  }

  rc = Fork();
  if (rc == 0) {
    above_user_space_access();
  }

  while(1)
  {
    Delay(100);
    TtyPrintf(0, "Parent woke up; going back to sleep\n");
  }
}