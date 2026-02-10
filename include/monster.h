#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"

/**
* @brief spawn a monster here
* @param position the spawn position of the monster
* @return NULL if out of entities, a pointer to the newly-created monster otherwise
*/
Entity* monster_new(GFC_Vector2D position);

#endif#pragma once
