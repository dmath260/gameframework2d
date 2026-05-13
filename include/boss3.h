#ifndef __BOSS3_H__
#define __BOSS3_H__

#include "entity.h"

/**
* @brief create a new boss3 entity
* @param position the spawn position of the boss
* @return NULL if out of entities, a pointer to the newly-created boss otherwise
*/
Entity* boss3_new(GFC_Vector2D position);

#endif