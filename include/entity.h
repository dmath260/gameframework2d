#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <SDL.h>

#include "gfc_text.h"

#include "gf2d_sprite.h"

typedef struct
{
	Uint8			_inuse; // no touchy
	GFC_TextLine	name;   //name of the entity for debugging purposes
	GFC_Vector2D	position;
	GFC_Vector2D	scale;
	float			rotation;
	Sprite			*sprite;
	float			frame;
}Entity;

/**
 * @brief initialize the entity subsystem
 * @param max the upper limit for concurrent entities
 */
void entity_manager_init(Uint32 max);

/**
 * @brief draws all entities
 */
void entity_manager_draw_all();

/**
 * @brief get a pointer to a free entity
 * @return NULL if out of entities, a pointer to a blank entity otherwise
 */
Entity* entity_new();

/**
 * @brief free an entity
 * @param self the entity to free
 * @note do not use the memory address again after calling this
 */
void entity_free(Entity* self);

#endif