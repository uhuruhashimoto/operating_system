#include <yuser.h>
#define PAGESIZE 0x2000

int main(void) {
    int *i = malloc(1000 * sizeof(int));
    if (i != NULL) {
        return 0;
    }
    return -1;
}