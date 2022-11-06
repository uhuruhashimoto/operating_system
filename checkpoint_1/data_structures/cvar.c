#include "cvar.h"
#include "queue.h"
#include "../kernel_start.h"

cvar_t* create_cvar(int cvar_id)
{
  cvar_t* new_cvar = malloc(sizeof (cvar_t));
  if (new_cvar == NULL) {
    TracePrintf(1, "CREATE_CVAR: Unable to malloc a new cvar\n");
  }
  new_cvar->id = cvar_id;
  new_cvar->blocked_queue = create_queue();
  new_cvar->next_cvar = NULL;

  if (new_cvar->blocked_queue == NULL) {
    TracePrintf(1, "CREATE_CVAR: Failed to allocate blocked queue\n");
    return NULL;
  }

  return new_cvar;
}

cvar_t* find_cvar(int cvar_id)
{

}

cvar_t* delete_cvar(cvar_t* cvar)
{

}