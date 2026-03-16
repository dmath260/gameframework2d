#include "simple_logger.h"

#include "monster_seeker.h"
#include "player.h"
#include "level.h"

void monster_seeker_free(Entity* self)
{
	if (!self) return;
	// clean up any seeker-exclusive stuff first
	monster_free(self);
}

void monster_seeker_think(Entity* self)
{
	GFC_Vector2D toPlayer = {0}, playerCenter = {0}, selfCenter = {0};
	MonsterData* data;
	Level* current_level;
	Uint32 tw, th, x_check, y_check, tile;
	Uint8 dir;

	current_level = get_current_level();
	if ((!self) || (!self->data) || !current_level) return;
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

	tw = current_level->tileDef->width;
	th = current_level->tileDef->height;
	dir = (self->animationData->FrameRow - 2) / 4;
	if ((1 - 2 * dir) * (data->player->position.x - self->position.x) > 800) // 25 tiles
	{
		self->velocity.x = 0; // can't see player from here, so don't move towards them
		return; // skip the rest of the checks
	}

	self->velocity.x = self->topSpeed * (1 - 2 * dir);
	self->thinkPos.x += self->velocity.x;
	x_check = self->velocity.x > 0 ?
		(int)((self->thinkPos.x + self->bounds.x + self->bounds.w + 4) / tw) :
		(int)((self->thinkPos.x + self->bounds.x - 4) / tw);
	y_check = (int)((self->thinkPos.y + self->bounds.y + self->bounds.h + 2) / th);
	tile = level_get_tile_index(current_level, x_check, y_check);

	if (tile != -1 && (check_bounds(self, 0) || current_level->tileMap[tile] <= 0))
	{
		self->velocity.x = 0;
		self->thinkPos.x = self->position.x;
	}
	else self->thinkPos.x -= self->velocity.x;
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
	self->topSpeed = 2;
	self->maxHealth = 6;
	self->attack = 2;
	self->think = monster_seeker_think;
	self->update = monster_seeker_update;
	self->touch = monster_seeker_touch;
	self->free = monster_seeker_free;
}