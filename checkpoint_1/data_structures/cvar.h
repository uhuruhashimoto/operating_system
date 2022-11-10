#ifndef CURRENT_CHUNGUS_CVAR_H
#define CURRENT_CHUNGUS_CVAR_H

#include <ykernel.h>
#include "queue.h"

typedef struct cvar_struct{
  int id;
  queue_t* blocked_queue;
  struct cvar_struct* next_cvar;
  struct cvar_struct* prev_cvar;
} cvar_t;

/*
 * Create a cvar with a particular id
 * The caller is trusted to not allocate a cvar with an id already on the cvar list
 */
cvar_t* create_cvar(int cvar_id);

/*
 * Find the next available id to allocate the cvar with
 */
cvar_t* create_cvar_any_id();

/*
 * Find a cvar with the given id
 */
cvar_t* find_cvar(int cvar_id);

/*
 * Delete a cvar
 */
int delete_cvar(cvar_t* cvar);

#endif //CURRENT_CHUNGUS_CVAR_H