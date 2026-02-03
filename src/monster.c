#include "simple_logger.h"

#include "monster.h"
#include "player.h"

typedef enum
{
	MS_Idle,
	MS_Hunt,
	MS_Attack,
	MS_Pain,
	MS_Die,
	MS_MAX
}MonsterStates;

typedef struct
{
	Entity* player;
	MonsterStates state;
} MonsterData;

void monster_free(Entity* self)
{
	MonsterData* data;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;
	//clean up anything I own that I asked for
	free(data);
}

void monster_think(Entity* self)
{
	GFC_Vector2D toPlayer = {0};
	MonsterData* data;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;
	if (!data->player) return;
	gfc_vector2d_sub(toPlayer, data->player->position, self->position);
	gfc_vector2d_normalize(&toPlayer);
	self->rotation = gfc_vector2d_angle(toPlayer) * GFC_RADTODEG + 135;
}

void monster_update(Entity* self)
{
	if (!self) return;

	self->frame += 0.04;
	if (self->frame >= 4.0) self->frame = 0;
}

Entity* monster_new(GFC_Vector2D position)
{
	Entity* self;
	MonsterData* data;
	self = entity_new();
	if (!self) return NULL;
	self->sprite = gf2d_sprite_load_all(
		"images/129s.png",
		64,
		64,
		4,
		0
	);
	data = gfc_allocate_array(sizeof(MonsterData), 1);
	if (data)
	{
		data->player = player_entity_get();
	}
	self->data = data;
	self->rotationCenter = gfc_vector2d(32, 32);
	self->topSpeed = 3;
	self->position = position;
	self->think = monster_think;
	self->update = monster_update;
	return self;
}