#include "simple_logger.h"

#include "gfc_input.h"

#include "player.h"

static Entity* thePlayer = NULL;

Entity* player_entity_get()
{
	return thePlayer;
}

void player_entity_think(Entity* self)
{
	GFC_Vector2D move = {0};
	if (!self) return;
	// Replace these with gfc_input_command_down if you can figure out how it works
	if (gfc_input_key_down("d"))
	{
		move.x += 1;
	}
	if (gfc_input_key_down("a"))
	{
		move.x -= 1;
	}
	if (gfc_input_key_down("s"))
	{
		move.y += 1;
	}
	if (gfc_input_key_down("w"))
	{
		move.y -= 1;
	}
	if ((move.x) || (move.y))
	{
		self->rotation = gfc_vector2d_angle(move) * GFC_RADTODEG;
		gfc_vector2d_normalize(&move);
		gfc_vector2d_scale(self->velocity, move, self->topSpeed);
	}
}

void player_entity_update(Entity* self)
{
	if (!self) return;

	self->frame += 0.04;
	if (self->frame >= 4.0) self->frame = 0;
}

Entity* player_entity_new(GFC_Vector2D position)
{
	Entity* self;
	self = entity_new();
	if (!self) return NULL;
	self->sprite = gf2d_sprite_load_all(
		"images/006s.png",
		64,
		64,
		4,
		0
	);
	self->rotationCenter = gfc_vector2d(32, 32);
	self->topSpeed = 3;
	self->position = position;
	self->think = player_entity_think;
	self->update = player_entity_update;
	thePlayer = self;
	return self;
}