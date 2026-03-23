#include "simple_logger.h"

#include "gfc_input.h"

#include "player.h"
#include "level.h"

Uint8 door_touch(Entity* self, Entity* other)
{
	if (!self || !other || other != player_entity_get()) return 0;
	Level* level;
	level = get_current_level();

	if (gfc_input_key_pressed("w")) {
		if (level->nextIsJSON) level_load(level->nextLevel);
		else level_load_bin(level->nextLevel);
	}
	return 1;
}

void door_think(Entity* self)
{
	return;
}

void door_update(Entity* self)
{
	entity_collision_test_world(self);
}

Entity* door_new(GFC_Vector2D position)
{
	Entity* self;
	self = entity_new();
	if (!self) return NULL;
	self->animDataFilePath = "images/door/doorAnimData.json";
	entity_load(self, "Door");
	self->animationData->FrameRow = 0;
	self->bounds = gfc_rect(10, 8, 44, 48);
	self->team = 2; // team 2 for items and such
	self->maxHealth = 0x7FFFFFFF;
	self->health = self->maxHealth;
	self->position = position;
	self->thinkPos = position;
	self->think = door_think;
	self->update = door_update;
	self->touch = door_touch;
	return self;
}