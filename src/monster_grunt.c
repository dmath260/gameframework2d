#include "simple_logger.h"

#include "monster_grunt.h"
#include "player.h"
#include "level.h"

void monster_grunt_free(Entity* self)
{
	if (!self) return;
	// clean up any grunt-exclusive stuff first
	monster_free(self);
}

void monster_grunt_think(Entity* self)
{
	//MonsterData* data;
	Level* current_level;
	Uint32 tw, th, x_check, y_check, tile;

	current_level = get_current_level();
	if ((!self) || (!self->data) || !current_level) return;
	//data = (MonsterData*)self->data;

	// do general monster thinking first
	monster_think(self);

	tw = current_level->tileDef->width;
	th = current_level->tileDef->height;
	self->velocity.x *= 2;
	self->thinkPos.x += self->velocity.x;
	x_check = self->velocity.x > 0 ?
		(int) ((self->thinkPos.x + self->bounds.x + self->bounds.w + 4) / tw) :
		(int) ((self->thinkPos.x + self->bounds.x - 4) / tw);
	y_check = (int) ((self->thinkPos.y + self->bounds.y + self->bounds.h + 2) / th);
	tile = level_get_tile_index(current_level, x_check, y_check);

	if (tile != -1 && (check_bounds(self, 0) || self->isGrounded && current_level->tileMap[tile] <= 0))
	{
		self->velocity.x *= -1;
		self->animationData->FrameRow = (self->animationData->FrameRow + 4) % 8; // 2 if left, 6 if right
		self->thinkPos.x = self->position.x;
	}
	else self->thinkPos.x -= self->velocity.x;
}

void monster_grunt_update(Entity* self)
{
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
	self->attack = 1;
	self->think = monster_grunt_think;
	self->update = monster_grunt_update;
	self->touch = monster_grunt_touch;
	self->free = monster_grunt_free;
}