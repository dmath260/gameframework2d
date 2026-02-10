#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

/**
* @brief get a pointer to the player if it exists
* @return a pointer to the player
*/
Entity* player_entity_get();

/**
* @brief create a new player entity
* @param position the spawn position of the player
* @return NULL if out of entities, a pointer to the newly-created player otherwise
*/
Entity* player_entity_new(GFC_Vector2D position);

#endif