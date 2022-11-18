#include <yuser.h>
#define PAGESIZE 0x2000

int main(void) {
  int size = 1000;
  int *i = malloc(size * sizeof(int));
  if (i != NULL) {
    TracePrintf(1, "Malloc Succeeded\n");
  } else {
    TracePrintf(1, "Malloc Failed\n");
  }

  TracePrintf(1, "Attempting to read from the end of the malloc-d array\n");
  TracePrintf(1, "%d\n", i[size-1]);
  TracePrintf(1, "Read Successful!\n");

  TracePrintf(1, "Attempting to free the malloc-d array\n");
  free(i);

  // do nothing
  while (1) {

  }
}