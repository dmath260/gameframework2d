#include "simple_logger.h"

#include "monster_flier.h"
#include "player.h"

void monster_flier_free(Entity* self)
{
	if (!self) return;
	// clean up any flier-exclusive stuff first
	monster_free(self);
}

void monster_flier_think(Entity* self)
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

void monster_flier_update(Entity* self)
{
	/*
	MonsterData* data;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;
	*/

	// do general monster updating first
	monster_update(self);
}

Uint8 monster_flier_touch(Entity* self, Entity* other)
{
	/*
	MonsterData* data;
	if (!self | !other) return;
	data = (MonsterData*) self->data;
	*/

	// do general monster touch checking first
	return monster_touch(self, other);
}

void monster_flier_populate(Entity *self)
{
	if (!self) return;
	self->animDataFilePath = "images/0041/0041AnimData.json";
	self->bounds = gfc_rect(-32, -56, 64, 112); // change these values later
	self->scale = gfc_vector2d(2, 2);
	self->rotationCenter = gfc_vector2d(16, 19);
	self->topSpeed = 3;
	self->think = monster_flier_think;
	self->update = monster_flier_update;
	self->touch = monster_flier_touch;
	self->free = monster_flier_free;
	return self;
}