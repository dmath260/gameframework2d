#include "simple_logger.h"

#include "gfc_input.h"

#include "player.h"

static Entity* thePlayer = NULL;

Entity* player_entity_get()
{
	return thePlayer;
}

typedef struct
{
	Uint16* animFrames;
	Uint8 frameRow;
	Uint8 frameCol;
	float frameCount;
	Uint8 framesPerRow;
	Uint8 framesPerCol;
} PlayerData;

void player_free(Entity* self)
{
	PlayerData* data;
	if ((!self) || (!self->data)) return;
	data = (PlayerData*)self->data;
	//clean up anything I own that I asked for
	free(data);
}

void player_entity_think(Entity* self)
{
	PlayerData* data;
	if ((!self) || (!self->data)) return;
	data = (PlayerData*)self->data;
	data->frameCount += 0.5;
	if (data->frameCount >= data->animFrames[data->frameCol + 1]) data->frameCol++;
	if (data->frameCol == data->framesPerRow) {
		data->frameCol = 0;
		data->frameCount = 0;
	}

	GFC_Vector2D move = {0};
	if (!self) return;
	// Replace these with gfc_input_command_down if you can figure out how it works
	if (gfc_input_key_down("d"))
	{
		move.x += 1;
	}
	if (gfc_input_key_down("a"))
	{
		move.x -= 1;
	}
	if (gfc_input_key_down("s"))
	{
		move.y += 1;
	}
	if (gfc_input_key_down("w"))
	{
		move.y -= 1;
	}
	if ((move.x) || (move.y))
	{
		int ang = ((int)(gfc_vector2d_angle(move) * GFC_RADTODEG) % 360);  // 0 for N, 1 for NE, 2 for E...
		if (ang % 45 > 22) ang += 45 - ang % 45; // rounding angle to nearest multiple of 45
		if (ang % 45 != 0) ang -= ang % 45; // ensures the right row of the spritesheet is chosen
		int dir = ang / 45;
		data->frameRow = (12 - dir) % 8; // row 0 for S (4), row 1 for SE (3), row 2 for E (2)...
		gfc_vector2d_normalize(&move);
		gfc_vector2d_scale(self->velocity, move, self->topSpeed);
	}
}

void player_entity_update(Entity* self)
{
	PlayerData* data;
	if ((!self) || (!self->data)) return;
	data = (PlayerData*)self->data;
	camera_center_on(self->position);

	self->frame = data->frameRow * data->framesPerRow + data->frameCol;
}

Entity* player_entity_new(GFC_Vector2D position)
{
	Entity* self;
	int i;
	PlayerData* data;
	self = entity_new();
	if (!self) return NULL;
	data = gfc_allocate_array(sizeof(PlayerData), 1);
	if (data)
	{
		data->frameCol = 0;
		data->frameRow = 0;
		data->frameCount = 0;
		data->framesPerCol = 8;
		data->framesPerRow = 7;
		data->animFrames = gfc_allocate_array(sizeof(int) * (data->framesPerRow + 1), 1);
		if (!data->animFrames) return;
		int frames[] = {38, 2, 2, 5, 3, 3, 2};
		for (i = 0; i < data->framesPerRow; i++)
		{
			data->animFrames[i + 1] = data->animFrames[i] + frames[i];
		}
	}
	self->data = data;
	self->sprite = gf2d_sprite_load_all(
		"images/0258/Idle-Anim.png",
		24,
		40,
		data->framesPerRow,
		0
	);
	self->rotationCenter = gfc_vector2d(12, 20);
	self->topSpeed = 3;
	self->position = position;
	self->think = player_entity_think;
	self->update = player_entity_update;
	self->scale = gfc_vector2d(2, 2);
	self->free = player_free;
	thePlayer = self;
	return self;
}