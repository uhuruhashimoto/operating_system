#include <yuser.h>
#define PAGESIZE 0x2000

int main(void) {
   while(1) {
      TracePrintf(1, "Hitting init...\n");
      Pause();
   }
   return 0;
}
