#include "cvar.h"
#include "queue.h"
#include "../kernel_start.h"
#include <ykernel.h>

cvar_t* create_cvar(int cvar_id)
{
  cvar_t* new_cvar = malloc(sizeof (cvar_t));
  if (new_cvar == NULL) {
    TracePrintf(1, "CREATE_CVAR: Unable to malloc a new cvar\n");
  }
  new_cvar->id = cvar_id;
  new_cvar->blocked_queue = create_queue();
  new_cvar->next_cvar = NULL;
  new_cvar->prev_cvar = NULL;

  if (new_cvar->blocked_queue == NULL) {
    TracePrintf(1, "CREATE_CVAR: Failed to allocate blocked queue\n");
    return NULL;
  }

  return new_cvar;
}

cvar_t* create_cvar_any_id() {
  int cvar_id = ++max_cvar_id;
  if (cvar_id > max_possible_cvar_id) {
    TracePrintf(1, "CREATE_CVAR_ANY_ID: Ran out of space to allocate new cvar ids\n");
    return NULL;
  }

  cvar_t* new_cvar = create_cvar(cvar_id);
  if (new_cvar == NULL) {
    TracePrintf(1, "CREATE_CVAR_ANY_ID: Failed to allocate new cvar\n");
    return NULL;
  }

  // stick the cvar in
  cvar_t* old_cvars = cvars;
  cvars = new_cvar;
  new_cvar->next_cvar = old_cvars;
  if (old_cvars != NULL) {
    old_cvars->prev_cvar = new_cvar;
  }

  return cvars;
}

cvar_t* find_cvar(int cvar_id)
{
  cvar_t* next_cvar = cvars;
  while (next_cvar != NULL) {
    if (next_cvar->id == cvar_id) {
      return next_cvar;
    }
  }

  return NULL;
}

cvar_t* delete_cvar(cvar_t* cvar)
{

}