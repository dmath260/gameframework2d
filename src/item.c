#include "simple_logger.h"

#include "item.h"
#include "player.h"

Uint8 item_touch(Entity* self, Entity* other)
{
	if (!self || !other || other != player_entity_get()) return 0;
	
	player_give_item(other, self->item);

	entity_free(self);
	return 1;
}

void item_think(Entity* self)
{
	self->velocity.y = 0.25; // floats down instead of falling
}

void item_update(Entity* self)
{
	if (self->item == IT_Invincible)
	{
		if (self->color.r >= 360) self->color.r -= 360;
		self->color.r += 0.5;
	}

	entity_collision_test_world(self);
}

Entity* item_new(GFC_Vector2D position, ItemTypes type)
{
	Entity* self;
	self = entity_new();
	if (!self) return NULL;
	self->item = type;
	self->animDataFilePath = "images/item/itemAnimData.json";
	entity_load(self, "Item");
	self->animationData->FrameRow = 0;
	self->bounds = gfc_rect(4, 4, 24, 24);
	switch (type)
	{
		case IT_Power:
		case IT_Invincible:
			self->color = gfc_color_hsl(0, 1, 0.5, 1); // used HSL so invincible can be rainbow
			break;
		case IT_Speed:
			self->color = gfc_color(0, 0.5, 1, 1); // lighter blue
			break;
		case IT_DoubleJump:
			self->color = GFC_COLOR_DARKGREEN;
			break;
		case IT_Hover:
			self->color = GFC_COLOR_YELLOW;
			break;
		default: self->color = GFC_COLOR_WHITE;
	}
	self->team = 2; // team 2 for items and such
	self->maxHealth = 0x7FFFFFFF;
	self->health = self->maxHealth;
	self->position = position;
	self->thinkPos = position;
	self->think = item_think;
	self->update = item_update;
	self->touch = item_touch;
	return self;
}