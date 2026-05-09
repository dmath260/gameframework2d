#include "simple_logger.h"

#include "gf2d_windows.h"

#include "monster_seeker.h"
#include "player.h"
#include "level.h"
#include "path.h"

void monster_seeker_free(Entity* self)
{
	if (!self) return;
	// clean up any seeker-exclusive stuff first
	monster_free(self);
}

void monster_seeker_think(Entity* self)
{
	// jump height for velocity.y -= 10: 195 px (~6 blocks)
	MonsterData* data;
	Level* current_level;
	Uint8 dir;
	GFC_Vector2D self_pos, next_pos;

	current_level = get_current_level();
	if ((!self) || (!self->data) || !current_level) return;
	data = (MonsterData*)self->data;

	// do general monster thinking first
	monster_think(self);

	if (!data->player) return;

	self_pos = gfc_vector2d(32 * roundf(self->position.x / 32), 32 * roundf(self->position.y / 32));
	next_pos = find_next(self->position, data->player->position, 0, 18, 0, 0);
	if (gfc_vector2d_compare(next_pos, self_pos))
	{
		return;
	}

	dir = next_pos.x < self_pos.x; // left if true, right if false
	if (next_pos.x == self_pos.x) dir = (self->animationData->FrameRow - 2) / 4;
	self->animationData->FrameRow = dir * 4 + 2;

	if (next_pos.x != self_pos.x) self->velocity.x = self->topSpeed * (1 - 2 * dir);
	else if (next_pos.y != self_pos.y) self->velocity.x = self->topSpeed * (1 - 2 * dir);
	//self->thinkPos.x += self->velocity.x;

	if (self_pos.y > next_pos.y && self->isGrounded)
	{
		self->velocity.y += self->impulse;
		self->isGrounded = 0;
		gf2d_windows_play_sound("jump2");
	}
}

void monster_seeker_update(Entity* self)
{
	/*
	MonsterData* data;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;
	*/

	// do general monster updating first
	monster_update(self);
}

Uint8 monster_seeker_touch(Entity* self, Entity* other)
{
	/*
	MonsterData* data;
	if (!self | !other) return;
	data = (MonsterData*) self->data;
	*/

	// do general monster touch checking first
	return monster_touch(self, other);
}

void monster_seeker_populate(Entity *self)
{
	if (!self) return;
	self->animDataFilePath = "images/0403/0403AnimData.json";
	self->bounds = gfc_rect(-37, -31, 66, 56); // change these values later
	self->scale = gfc_vector2d(2, 2);
	self->rotationCenter = gfc_vector2d(20, 18);
	self->impulse = -10;
	self->topSpeed = 1.5;
	self->maxHealth = 6;
	self->attack = 2;
	self->think = monster_seeker_think;
	self->update = monster_seeker_update;
	self->touch = monster_seeker_touch;
	self->free = monster_seeker_free;
}