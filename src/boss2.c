#include "simple_logger.h"

#include "gf2d_windows.h"

#include "boss2.h"
#include "door.h"
#include "bullet.h"
#include "player.h"

typedef struct
{
	Entity* player;
	Uint8 phase;
} Boss2Data;

void boss2_think(Entity* self)
{
	Boss2Data* data;
	Uint8 i;

	if ((!self) || (!self->data)) return;
	data = (Boss2Data*)self->data;

	switch (data->phase) {
		case 0:
			self->cooldown++; // don't attack just yet
			if (self->iFrames <= 5)
			{
				// transition to Phase 1
				self->iFrames = 255;
				data->phase = 1;
				self->thinkPos = gfc_vector2d(32 * 46, 32 * 37);
				gf2d_windows_play_sound("warp");
			}
			break;
		case 1:
			if (self->health <= 2 * self->maxHealth / 3)
			{
				data->phase = 2;
				self->iFrames = 255;
				self->thinkPos = gfc_vector2d(32 * 98, 32 * 32);
				gf2d_windows_play_sound("warp");
			}
			else if (self->iFrames <= 5 && (self->position.x - data->player->position.x) > 480)
			{
				self->iFrames = 255;
				self->color = GFC_COLOR_LIGHTGREY;
			}
			else
			{
				if (self->iFrames == 0) self->color = GFC_COLOR_WHITE;
			}
			break;
		case 2:
			if (self->health <= 1 * self->maxHealth / 3)
			{
				data->phase = 3;
				self->iFrames = 255;
				self->thinkPos = gfc_vector2d(32 * 175, 32 * 11);
				gf2d_windows_play_sound("warp");
			}
			else if (self->iFrames <= 5 && (self->position.x - data->player->position.x) > 480)
			{
				self->iFrames = 255;
				self->color = GFC_COLOR_LIGHTGREY;
			}
			else
			{
				if (self->iFrames == 0) self->color = GFC_COLOR_WHITE;
			}
			break;
		case 3:
			if (self->iFrames <= 5 && (self->position.x - data->player->position.x) > 480)
			{
				self->iFrames = 255;
				self->color = GFC_COLOR_LIGHTGREY;
			}
			else
			{
				if (self->iFrames == 0) self->color = GFC_COLOR_WHITE;
			}
			break;
		default:
			entity_free(self);
	}

	for (i = 0; i < data->phase; i++)
	{
		self->cooldown--;
	}
	if (self->cooldown <= 0)
	{
		self->cooldown = self->maxCooldown;
		bullet_new(
			gfc_vector2d(
				self->position.x - self->bounds.w / 2,
				self->position.y - self->bounds.h / 2 * gfc_crandom()
			),
			GFC_COLOR_RED,
			self->team,
			1,
			self->attack
		);
	}

	if (self) self->data = data;
}

void boss2_update(Entity* self)
{
	return;
}

void boss2_free(Entity* self)
{
	if (self->health > 0) return;
	door_new(self->position);
	gf2d_windows_play_sound("boss_ko");
}

Entity* boss2_new(GFC_Vector2D position)
{
	Entity* self;
	Boss2Data* data;
	self = entity_new();
	if (!self) return NULL;
	data = gfc_allocate_array(sizeof(Boss2Data), 1);
	if (data)
	{
		data->player = player_entity_get();
	}
	self->data = data;
	populate_entity(self, "def/boss2.def");
	self->health = self->maxHealth;
	self->iFrames = 255;
	self->color = GFC_COLOR_LIGHTGREY;
	self->position = position;
	self->thinkPos = position;
	self->cooldown = self->maxCooldown;
	self->think = boss2_think;
	self->update = boss2_update;
	self->free = boss2_free;
	return self;
}