#include "simple_logger.h"

#include "gfc_input.h"

#include "player.h"
#include "camera.h"

typedef enum
{
	PL_Null,
	PL_Idle,
	PL_Walk,
	PL_Attack,
	PL_Pain,
	PL_Die,
	PL_MAX
}PlayerStates;

typedef struct
{
	PlayerStates state;
} PlayerData;

static Entity* thePlayer = NULL;

Entity* player_entity_get()
{
	return thePlayer;
}

void player_free(Entity* self)
{
	PlayerData* data;
	if ((!self) || (!self->data)) return;
	data = (PlayerData*)self->data;
	//clean up anything I own that I asked for
	free(data);
}

char* state_to_str(Entity* self)
{
	if (!self || !self->data) return;
	PlayerStates state;
	PlayerData* data;
	data = (PlayerData*)self->data;
	state = data->state;
	switch (state)
	{
	case PL_Idle: return "Idle";
	case PL_Walk: return "Walk";
	case PL_Attack: return "Attack";
	case PL_Pain: return "Pain";
	case PL_Die: return "Faint";
	default: return "Idle";
	}
}

void set_player_state(Entity* self, PlayerStates state)
{
	if (!self || !self->data) return;
	PlayerData* data;
	data = (PlayerData*)self->data;
	if (data->state == state) return;
	data->state = state;
	self->data = data;
	entity_load(self, state_to_str(self));
}

void player_entity_think(Entity* self)
{
	PlayerData* pdata;
	if ((!self) || (!self->data)) return;
	pdata = (PlayerData*)self->data;
	if (!self->animationData) return;
	AnimData* adata;
	adata = (AnimData*)self->animationData;

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
		adata->FrameRow = (12 - dir) % 8; // row 0 for S (4), row 1 for SE (3), row 2 for E (2)...
		gfc_vector2d_normalize(&move);
		gfc_vector2d_scale(self->velocity, move, self->topSpeed);
		set_player_state(self, PL_Walk);
	}
	else
	{
		set_player_state(self, PL_Idle);
	}
}

void player_entity_update(Entity* self)
{
	if (!self) return;
	camera_center_on(self->position);
}

Entity* player_entity_new(GFC_Vector2D position)
{
	Entity* self;
	self = entity_new();
	if (!self) return NULL;
	PlayerData* data;
	data = gfc_allocate_array(sizeof(PlayerData), 1);
	self->data = data;
	self->animDataFilePath = "images/0258/0258AnimData.json";
	set_player_state(self, PL_Idle);
	self->rotationCenter = gfc_vector2d(12, 16);
	self->bounds = gfc_rect(-32, -40, 64, 80); // change these values later AND move to set_player_state
	self->topSpeed = 3;
	self->position = position;
	self->think = player_entity_think;
	self->update = player_entity_update;
	self->scale = gfc_vector2d(2, 2);
	self->free = player_free;
	thePlayer = self;
	return self;
}