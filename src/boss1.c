#include "simple_logger.h"

#include "boss1.h"
#include "door.h"
#include "bullet.h"
#include "player.h"

typedef struct
{
	Entity* player;
	Uint8 phase;
} Boss1Data;

void boss1_think(Entity* self)
{
	Boss1Data* data;
	Uint8 i;

	if ((!self) || (!self->data)) return;
	data = (Boss1Data*)self->data;

	switch (data->phase) {
		case 0:
			self->cooldown++; // don't attack just yet
			if (self->iFrames <= 5)
			{
				// transition to Phase 1
				self->iFrames = 255;
				data->phase = 1;
				self->thinkPos = gfc_vector2d(32 * 46, 32 * 37);
			}
			break;
		case 1:
			if (self->health <= 2 * self->maxHealth / 3)
			{
				data->phase = 2;
				self->iFrames = 255;
				self->thinkPos = gfc_vector2d(32 * 98, 32 * 32);
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

	/*
	self->cooldown--;
	if (!self->cooldown)
	{
		self->cooldown = self->maxCooldown;
		for (i = 0; i < data->phase; i++)
		{
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
	}
	*/

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

void boss1_update(Entity* self)
{
	return;
}

void boss1_free(Entity* self)
{
	door_new(self->position);
}

Entity* boss1_new(GFC_Vector2D position)
{
	Entity* self;
	Boss1Data* data;
	self = entity_new();
	if (!self) return NULL;
	data = gfc_allocate_array(sizeof(Boss1Data), 1);
	if (data)
	{
		data->player = player_entity_get();
	}
	self->data = data;
	self->animDataFilePath = "images/0386/0386AnimData.json";
	entity_load(self, "Walk");
	self->animationData->FrameRow = 6;
	self->rotationCenter = gfc_vector2d(12, 16);
	self->bounds = gfc_rect(-40, -70, 130, 150); // change these values later AND move to set_player_state
	self->gravity = 0;
	self->topSpeed = 20;
	self->maxHealth = 60;
	self->health = self->maxHealth;
	self->attack = 3;
	self->maxIFrames = 90;
	self->iFrames = 255;
	self->color = GFC_COLOR_LIGHTGREY;
	self->team = 1; // team 1 for monsters and bosses
	self->position = position;
	self->thinkPos = position;
	self->scale = gfc_vector2d(4, 4);
	self->maxCooldown = 180;
	self->cooldown = self->maxCooldown;
	self->think = boss1_think;
	self->update = boss1_update;
	self->free = boss1_free;
	return self;
}