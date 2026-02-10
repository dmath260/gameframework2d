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
	Uint16* animFrames;
	Uint8 frameRow;
	Uint8 frameCol;
	float frameCount;
	Uint8 framesPerRow;
	Uint8 framesPerCol;
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
	data->frameCount += 0.5;
	if (data->frameCount >= data->animFrames[data->frameCol + 1]) data->frameCol++;
	if (data->frameCol == data->framesPerRow) {
		data->frameCol = 0;
		data->frameCount = 0;
	}

	if (!data->player) return;
	gfc_vector2d_sub(toPlayer, data->player->position, self->position);
	gfc_vector2d_normalize(&toPlayer);
	int ang = ((int)(gfc_vector2d_angle(toPlayer) * GFC_RADTODEG) % 360);  // 0 for N, 1 for NE, 2 for E...
	if (ang % 45 > 22) ang += 45 - ang % 45; // rounding angle to nearest multiple of 45
	if (ang % 45 != 0) ang -= ang % 45; // ensures the right row of the spritesheet is chosen
	int dir = ang / 45;
	data->frameRow = (12 - dir) % 8; // row 0 for S (4), row 1 for SE (3), row 2 for E (2)...
}

void monster_update(Entity* self)
{
	MonsterData* data;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;

	self->frame = data->frameRow * data->framesPerRow + data->frameCol;
}

Entity* monster_new(GFC_Vector2D position)
{
	Entity* self;
	MonsterData* data;
	int i;
	self = entity_new();
	if (!self) return NULL;
	data = gfc_allocate_array(sizeof(MonsterData), 1);
	if (data)
	{
		data->player = player_entity_get();
		data->frameCol = 0;
		data->frameRow = 0;
		data->frameCount = 0;
		data->framesPerCol = 8;
		data->framesPerRow = 4;
		data->animFrames = gfc_allocate_array(sizeof(int) * (data->framesPerRow + 1), 1);
		if (!data->animFrames) return;
		int frames[] = { 20, 14, 20, 14 };
		for (i = 0; i < data->framesPerRow; i++)
		{
			data->animFrames[i + 1] = data->animFrames[i] + frames[i];
		}
	}
	self->sprite = gf2d_sprite_load_all(
		"images/0399/Idle-Anim.png",
		32,
		32,
		data->framesPerRow,
		0
	);
	self->scale = gfc_vector2d(2, 2);
	self->data = data;
	self->rotationCenter = gfc_vector2d(32, 32);
	self->topSpeed = 3;
	self->position = position;
	self->think = monster_think;
	self->update = monster_update;
	return self;
}