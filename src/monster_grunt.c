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
	self->velocity.x *= 2;
	self->thinkPos.x += self->velocity.x;
	if (check_bounds(self, 0) || (self->velocity.y > 0 && self->isGrounded))
	{
		self->velocity.x *= -1;
		self->animationData->FrameRow = (self->animationData->FrameRow + 4) % 8; // 2 if left, 6 if right
		if (self->velocity.y > 0)
		{
			self->velocity.y = 0;
			self->thinkPos.y = self->position.y;
		}
	}
	self->thinkPos.x -= self->velocity.x;
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
	self->topSpeed = 1;
	self->velocity.x = self->topSpeed / 2;
	self->maxHealth = 2;
	self->think = monster_grunt_think;
	self->update = monster_grunt_update;
	self->touch = monster_grunt_touch;
	self->free = monster_grunt_free;
}