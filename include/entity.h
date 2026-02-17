#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <SDL.h>

#include "gfc_text.h"
#include "gfc_shape.h"

#include "gf2d_sprite.h"

#include "animation.h"

typedef struct Entity_S
{
	Uint8			_inuse;						//no touchy
	GFC_TextLine	name;						//name of the entity for debugging purposes
	GFC_Vector2D	position;
	GFC_Vector2D	velocity;
	float			topSpeed;
	GFC_Vector2D	scale;
	GFC_Vector2D	rotationCenter;
	float			rotation;
	GFC_Rect		bounds;						//bounding box
	float			frame;
	void (* think)	(struct Entity_S *self);	//called every frame if defined for the entity
	void (* update)	(struct Entity_S *self);	//called every frame if defined for the entity
	void (* free)	(struct Entity_S *self);	//called when the entity is freed
	Uint8 (* touch)	(struct Entity_S *self, struct Entity_S *other);
	// return 1 if you want to stop movement
	void			*data;						//used for entity specific data
	AnimData		*animationData;				//data for animating the entity
	char			*animDataFilePath;			//file path to JSON for animation data
}Entity;

/**
 * @brief initialize the entity subsystem
 * @param max the upper limit for concurrent entities
 */
void entity_manager_init(Uint32 max);

/**
 * @brief draws an entity
 * @param self the entity to draw
 */
void entity_draw(Entity *self);

/**
 * @brief draws all entities
 */
void entity_manager_draw_all();

/**
* @brief check if an entity is colliding with any other entity
* @param self the entity to check
*/
Uint8 entity_collision_test_world(Entity* self);

/**
 * @brief all active entities run their think
 */
void entity_manager_think_all();

/**
 * @brief all active entities run their update
 */
void entity_manager_update_all();

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

/**
* @brief load animation data from a JSON file to an existing entity
* @param ent the entity to load the animation data to
* @param state the state of the entity for loading animations
*/
void entity_load(Entity* ent, char* state);

/**
 * @brief kills a random entity other than the player (assumes player is first entity initialized)
 */
void entity_manager_kill_random();

#endif