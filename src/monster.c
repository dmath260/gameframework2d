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
	GFC_Vector2D toPlayer = {0}, playerCenter = {0}, selfCenter = {0};
	MonsterData* data;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;

	if (!data->player) return;
	gfc_vector2d_add(playerCenter, data->player->position, data->player->rotationCenter);
	gfc_vector2d_add(selfCenter, self->position, self->rotationCenter);
	gfc_vector2d_sub(toPlayer, playerCenter, selfCenter);
	gfc_vector2d_normalize(&toPlayer);
	int ang = ((int)(gfc_vector2d_angle(toPlayer) * GFC_RADTODEG) % 360);  // 0 for N, 1 for NE, 2 for E...
	if (ang % 45 > 22) ang += 45 - ang % 45; // rounding angle to nearest multiple of 45
	if (ang % 45 != 0) ang -= ang % 45; // ensures the right row of the spritesheet is chosen
	int dir = ang / 45;
	self->animationData->FrameRow = (12 - dir) % 8; // row 0 for S (4), row 1 for SE (3), row 2 for E (2)...
}

void monster_update(Entity* self)
{
	/*
	MonsterData* data;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;
	*/
}

Entity* monster_new(GFC_Vector2D position)
{
	Entity* self;
	MonsterData* data;
	self = entity_new();
	if (!self) return NULL;
	data = gfc_allocate_array(sizeof(MonsterData), 1);
	if (data)
	{
		data->player = player_entity_get();
	}
	self->animDataFilePath = "images/0399/0399AnimData.json";
	entity_load(self, "Idle");
	self->scale = gfc_vector2d(2, 2);
	self->data = data;
	self->rotationCenter = gfc_vector2d(16, 16);
	self->topSpeed = 3;
	self->position = position;
	self->think = monster_think;
	self->update = monster_update;
	return self;
}