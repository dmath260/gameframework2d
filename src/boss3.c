#include "simple_logger.h"

#include "gf2d_windows.h"

#include "boss3.h"
#include "cutscene.h"
#include "door.h"
#include "bullet.h"
#include "player.h"

typedef struct
{
	Entity* player;
	Uint8 phase;
} Boss3Data;

void boss3_think(Entity* self)
{
	GFC_Vector2D toPlayer = { 0 }, playerCenter = { 0 }, selfCenter = { 0 };
	Boss3Data* data;
	Uint8 dir, i;

	if ((!self) || (!self->data)) return;
	data = (Boss3Data*)self->data;
	if (!data->player) return;
	gfc_vector2d_add(playerCenter, data->player->position, data->player->rotationCenter);
	gfc_vector2d_add(selfCenter, self->position, self->rotationCenter);
	gfc_vector2d_sub(toPlayer, playerCenter, selfCenter);
	gfc_vector2d_normalize(&toPlayer);
	if (toPlayer.x >= 0) self->animationData->FrameRow = 2;
	else self->animationData->FrameRow = 6;

	switch (data->phase) {
		case 0:
			if (self->iFrames <= 5)
			{
				// transition to Phase 1
				self->iFrames = 255;
				data->phase = 1;
				self->thinkPos = gfc_vector2d(32 * 35, 32 * 174);
				gf2d_windows_play_sound("warp");
			}
			break;
		case 1:
			if (self->health <= 2 * self->maxHealth / 3)
			{
				data->phase = 2;
				self->iFrames = 255;
				self->thinkPos = gfc_vector2d(32 * 35, 32 * 130);
				gf2d_windows_play_sound("warp");
			}
			else if (self->iFrames <= 5 && data->player->position.y - self->position.y > 320)
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
				self->thinkPos = gfc_vector2d(32 * 35, 32 * 41);
				gf2d_windows_play_sound("warp");
			}
			else if (self->iFrames <= 5 && data->player->position.y - self->position.y > 320)
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
			if (self->iFrames <= 5 && data->player->position.y - self->position.y > 320)
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
		dir = (self->animationData->FrameRow - 2) / 4;
		self->cooldown = self->maxCooldown;
		if (data->player->position.y - self->position.y <= 320) bullet_new(
			gfc_vector2d(
				self->position.x - self->bounds.w / 2,
				self->position.y - self->bounds.h / 2 * gfc_crandom()
			),
			GFC_COLOR_RED,
			self->team,
			dir,
			self->attack
		);
	}

	if (self) self->data = data;
}

void boss3_update(Entity* self)
{
	return;
}

void boss3_free(Entity* self)
{
	if (self->health > 0) return;
	door_new(self->position);
	gf2d_windows_play_sound("boss_ko");
}

Entity* boss3_new(GFC_Vector2D position)
{
	Entity* self;
	Boss3Data* data;
	self = entity_new();
	if (!self) return NULL;
	data = gfc_allocate_array(sizeof(Boss3Data), 1);
	if (data)
	{
		data->player = player_entity_get();
	}
	self->data = data;
	populate_entity(self, "def/boss3.def");
	self->health = self->maxHealth;
	self->iFrames = 255;
	self->color = GFC_COLOR_LIGHTGREY;
	self->position = position;
	self->thinkPos = position;
	self->cooldown = self->maxCooldown;
	self->think = boss3_think;
	self->update = boss3_update;
	self->free = boss3_free;
	cutscene_load("def/cutscenes/boss3.cut");
	return self;
}