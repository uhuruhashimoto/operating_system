#include "cvar.h"
#include "queue.h"
#include "../kernel_start.h"
#include <ykernel.h>

/*
 * Create a cvar with a particular id
 * The caller is trusted to not allocate a cvar with an id already on the cvar list
 */
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

/*
 * Find the next available id to allocate the cvar with
 */
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

  // stick the cvar in the list
  cvar_t* old_cvars = cvars;
  cvars = new_cvar;
  new_cvar->next_cvar = old_cvars;
  if (old_cvars != NULL) {
    old_cvars->prev_cvar = new_cvar;
  }

  return cvars;
}

/*
 * Find a cvar with the given id
 */
cvar_t* find_cvar(int cvar_id)
{
  cvar_t* next_cvar = cvars;
  while (next_cvar != NULL) {
//    TracePrintf(1, "cvar: %d\n", next_cvar->id);
    if (next_cvar->id == cvar_id) {
      return next_cvar;
    }
    next_cvar = next_cvar->next_cvar;
  }

  return NULL;
}

/*
 * Delete a cvar; we assume blocked_queue to already be empty
 */
int delete_cvar(cvar_t* cvar)
{
  if (cvar != NULL) {
    free(cvar->blocked_queue);
  }
  free(cvar);
  return SUCCESS;
}