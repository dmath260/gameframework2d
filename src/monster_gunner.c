#include "simple_logger.h"

#include "monster_gunner.h"
#include "bullet.h"
#include "player.h"

void monster_gunner_free(Entity* self)
{
	if (!self) return;
	// clean up any gunner-exclusive stuff first
	monster_free(self);
}

void monster_gunner_think(Entity* self)
{
	GFC_Vector2D toPlayer = {0}, playerCenter = {0}, selfCenter = {0};
	MonsterData* data;
	Uint8 dir;
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
	self->cooldown--;
	if (!self->cooldown)
	{
		self->cooldown = self->maxCooldown;
		dir = (self->animationData->FrameRow - 2) / 4;
		if ((1 - 2 * dir) * (data->player->position.x - self->position.x) <= 576) // 25 tiles
			bullet_new(
				gfc_vector2d(self->position.x + 25 * (1 - 2 * dir), self->position.y - 12),
				gfc_color8(255, 0, 0, 255),
				self->team,
				dir,
				self->attack
			);
	}
}

void monster_gunner_update(Entity* self)
{
	/*
	MonsterData* data;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;
	*/

	// do general monster updating first
	monster_update(self);
}

Uint8 monster_gunner_touch(Entity* self, Entity* other)
{
	/*
	MonsterData* data;
	if (!self | !other) return;
	data = (MonsterData*) self->data;
	*/

	// do general monster touch checking first
	return monster_touch(self, other);
}

void monster_gunner_populate(Entity *self)
{
	if (!self) return;
	self->animDataFilePath = "images/0069/0069AnimData.json";
	self->bounds = gfc_rect(-16, -29, 32, 42); // change these values later
	self->scale = gfc_vector2d(2, 2);
	self->rotationCenter = gfc_vector2d(12, 17);
	self->maxHealth = 6;
	self->attack = 2;
	self->maxCooldown = 90;
	self->cooldown = self->maxCooldown;
	self->think = monster_gunner_think;
	self->update = monster_gunner_update;
	self->touch = monster_gunner_touch;
	self->free = monster_gunner_free;
}