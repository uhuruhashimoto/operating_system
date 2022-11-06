#ifndef CURRENT_CHUNGUS_CVAR_H
#define CURRENT_CHUNGUS_CVAR_H

#include <ykernel.h>
#include "queue.h"

typedef struct cvar_struct{
  int id;
  queue_t* blocked_queue;
  struct cvar_struct next_cvar;
} cvar_t;

cvar_t* create_cvar(int cvar_id);

cvar_t* find_cvar(int cvar_id);

cvar_t* delete_cvar(cvar_t* cvar);

#endif //CURRENT_CHUNGUS_CVAR_H