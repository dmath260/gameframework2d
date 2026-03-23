#ifndef __BOSS1_H__
#define __BOSS1_H__

#include "entity.h"

/**
* @brief create a new boss1 entity
* @param position the spawn position of the boss
* @return NULL if out of entities, a pointer to the newly-created boss otherwise
*/
Entity* boss1_new(GFC_Vector2D position);

#endif