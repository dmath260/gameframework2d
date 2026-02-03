#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

/**
* @brief get a pointer to the player if it exists
*/
Entity* player_entity_get();

Entity* player_entity_new(GFC_Vector2D position);

#endif