#ifndef __BULLET_H__
#define __BULLET_H__

#include "entity.h"

/**
* @brief create a bullet
* @param position the spawn position of the bullet
* @param color the color of the bullet
* @param team the team of the entity that spawned it
* @param 0 for right, 1 for left
* @param the amount of damage the bullet does to opponents
* @return NULL if out of entities, a pointer to the newly-created bullet otherwise
*/
Entity* bullet_new(GFC_Vector2D position, GFC_Color color, Uint8 team, Uint8 dir, float power);

#endif