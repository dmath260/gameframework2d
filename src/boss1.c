#include "simple_logger.h"

#include "gf2d_windows.h"

#include "boss1.h"
#include "door.h"
#include "cutscene.h"
#include "bullet.h"
#include "player.h"

Uint8 _boss1_counter;
GFC_Rect _edges;

typedef struct
{
	Entity* player;
	Uint8 id;
	Uint8 dir;
} Boss1Data;

void boss1_think(Entity* self)
{
	Boss1Data* data;
	GFC_Vector2D direction;

	if ((!self) || (!self->data)) return;
	data = (Boss1Data*)self->data;

	self->cooldown--;
	if (!self->cooldown)
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

	if (data->dir == 0) direction = gfc_vector2d(1, 1);
	else if (data->dir == 1) direction = gfc_vector2d(-1, 1);
	else if (data->dir == 2) direction = gfc_vector2d(-1, -1);
	else direction = gfc_vector2d(1, -1);
	self->velocity.x = direction.x * self->topSpeed;
	self->velocity.y = direction.y * self->topSpeed;
	self->thinkPos.y += self->velocity.y;

	if (self->thinkPos.x + self->velocity.x >= _edges.w) data->dir = 1;
	else if (self->thinkPos.y >= _edges.h) data->dir = 2;
	else if (self->thinkPos.x + self->velocity.x <= _edges.x) data->dir = 3;
	else if (self->thinkPos.y <= _edges.y) data->dir = 0;
}

void boss1_update(Entity* self)
{
	return;
}

void boss1_free(Entity* self)
{
	_boss1_counter--;
	if (self->health > 0) return;
	if (_boss1_counter == 0) door_new(self->position);
	gf2d_windows_play_sound("boss_ko");
}

Entity* boss1_new(GFC_Vector2D position)
{
	Entity* self;
	Boss1Data* data;
	self = entity_new();
	if (!self) return NULL;
	if (_boss1_counter <= 0 || _boss1_counter >= 4) _boss1_counter = 1;
	else _boss1_counter++;
	populate_entity(self, "def/boss1.def");
	self->health = self->maxHealth;
	self->position = position;
	self->thinkPos = position;

	data = gfc_allocate_array(sizeof(Boss1Data), 1);
	if (data)
	{
		data->player = player_entity_get();
		data->id = _boss1_counter;
		switch (data->id) {
		case 1:
			_edges.y = position.y; // top edge
			self->cooldown = self->maxCooldown / 2;
			data->dir = 0; // down and right
			break;
		case 2:
			_edges.x = position.x; // left edge
			self->cooldown = self->maxCooldown / 4;
			data->dir = 3; // up and right
			break;
		case 3:
			_edges.w = position.x; // right edge
			self->cooldown = self->maxCooldown * 3 / 4;
			data->dir = 1; // down and left
			break;
		case 4:
			_edges.h = position.y; // bottom edge
			self->cooldown = self->maxCooldown;
			data->dir = 2; // up and left
		}
	}
	self->data = data;

	self->think = boss1_think;
	self->update = boss1_update;
	self->free = boss1_free;
	cutscene_load("def/cutscenes/boss1.cut");
	return self;
}