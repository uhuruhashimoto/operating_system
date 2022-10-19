#include <ykernel.h>

/*
* Switches kernel context, being careful to save special registers
*/
KernelContext *KCSwitch( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p);

/*
* Copies the kernel context from *kc_in to the *new_pcb_p, and copies
* kernel stack to unused, allocated frames. Returns kc_in.
*/
KernelContext *KCCopy( KernelContext *kc_in, void *new_pcb_p,void *not_used);