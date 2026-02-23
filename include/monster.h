#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "entity.h"

typedef enum
{
	MS_Idle,
	MS_Hunt,
	MS_Attack,
	MS_Pain,
	MS_Die,
	MS_MAX
}MonsterStates;

typedef struct
{
	Entity* player;
	MonsterStates state;
} MonsterData;

typedef enum
{
	MT_Grunt,
	MT_Seeker,
	MT_Gunner,
	MT_Flier,
	MT_ImmortalSnail,
	MT_MAX
}MonsterTypes;

/**
* @brief spawn a monster here
* @param position the spawn position of the monster
* @param type the type of monster to spawn
* @return NULL if out of entities, a pointer to the newly-created monster otherwise
*/
Entity* monster_new(GFC_Vector2D position, MonsterTypes type);

/**
* @brief free a monster
* @param self the monster to free
*/
void monster_free(Entity* self);

/**
* @brief run think for a monster
* @param self the monster that's thinking
*/
void monster_think(Entity* self);

/**
* @brief run update for a monster
* @param self the monster to update
*/
void monster_update(Entity* self);

/**
* @brief check if monster is touching another entity
* @param self the monster to check
* @param other the other entity to check
* @return 0 if error, 1 otherwise
*/
Uint8 monster_touch(Entity* self, Entity* other);

#endif#pragma once
