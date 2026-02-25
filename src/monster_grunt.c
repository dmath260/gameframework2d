#include "simple_logger.h"

#include "monster_grunt.h"
#include "player.h"

void monster_grunt_free(Entity* self)
{
	if (!self) return;
	// clean up any grunt-exclusive stuff first
	monster_free(self);
}

void monster_grunt_think(Entity* self)
{
	GFC_Vector2D toPlayer = {0}, playerCenter = {0}, selfCenter = {0};
	MonsterData* data;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;

	// do general monster thinking first
	monster_think(self);

	if (!data->player) return;
	gfc_vector2d_add(playerCenter, data->player->position, data->player->rotationCenter);
	gfc_vector2d_add(selfCenter, self->position, self->rotationCenter);
	gfc_vector2d_sub(toPlayer, playerCenter, selfCenter);
	gfc_vector2d_normalize(&toPlayer);
	if (toPlayer.x >= 0) self->animationData->FrameRow = 2;
	else self->animationData->FrameRow = 6;
}

void monster_grunt_update(Entity* self)
{
	/*
	MonsterData* data;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;
	*/

	// do general monster updating first
	monster_update(self);
}

Uint8 monster_grunt_touch(Entity* self, Entity* other)
{
	/*
	MonsterData* data;
	if (!self | !other) return;
	data = (MonsterData*) self->data;
	*/

	// do general monster touch checking first
	return monster_touch(self, other);
}

void monster_grunt_populate(Entity *self)
{
	if (!self) return;
	self->animDataFilePath = "images/0399/0399AnimData.json";
	self->bounds = gfc_rect(-26, -23, 52, 36); // change these values later
	self->scale = gfc_vector2d(2, 2);
	self->rotationCenter = gfc_vector2d(16, 16);
	self->topSpeed = 3;
	self->think = monster_grunt_think;
	self->update = monster_grunt_update;
	self->touch = monster_grunt_touch;
	self->free = monster_grunt_free;
}