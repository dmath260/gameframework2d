#include "simple_logger.h"

#include "player.h"

Entity* player_entity_new(GFC_Vector2D position)
{
	Entity* self;
	self = entity_new();
	if (!self) return NULL;
	self->sprite = gf2d_sprite_load_all(
		"images/ed210_top.png",
		128,
		128,
		16,
		0
	);
	self->position = position;
	return self;
}