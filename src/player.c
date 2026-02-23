#include "simple_logger.h"

#include "gfc_input.h"

#include "player.h"
#include "camera.h"

typedef enum
{
	PS_Null,
	PS_Idle,
	PS_Walk,
	PS_Jump,
	PS_Attack,
	PS_Pain,
	PS_Die,
	PS_MAX
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
	case PS_Idle: return "Idle";
	case PS_Walk: return "Walk";
	case PS_Jump: return "Hop";
	case PS_Attack: return "Attack";
	case PS_Pain: return "Pain";
	case PS_Die: return "Faint";
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
	if (!self || !self->data) return;

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
	if (gfc_input_key_down("w") && self->isGrounded)
	{
		self->velocity.y -= 10;
		self->isGrounded = 0;
	}
	if ((gfc_input_key_down("LSHIFT") || gfc_input_key_down("RSHIFT")) && self->isGrounded)
	{
		// Has to be grounded because sprinting in midair makes no sense
		self->speedMult = 2.0;
	}
	else if (!gfc_input_key_down("LSHIFT") && !gfc_input_key_down("RSHIFT"))
	{
		// However, if the player was sprinting before jumping, don't kill their horizontal momentum
		self->speedMult = 1.0;
	}
	if (!self->isGrounded) set_player_state(self, PS_Jump);
	if (move.x)
	{
		gfc_vector2d_normalize(&move);
		self->velocity.x = move.x * self->topSpeed * self->speedMult;
		if (move.x >= 0) self->animationData->FrameRow = 2;
		else self->animationData->FrameRow = 6;
		if (self->isGrounded) set_player_state(self, PS_Walk);
	}
	if (!move.x && self->isGrounded)
	{
		set_player_state(self, PS_Idle);
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
	set_player_state(self, PS_Idle);
	self->rotationCenter = gfc_vector2d(12, 16);
	self->bounds = gfc_rect(-24, -40, 64, 80); // change these values later AND move to set_player_state
	self->topSpeed = 3;
	self->speedMult = 1;
	self->position = position;
	self->think = player_entity_think;
	self->update = player_entity_update;
	self->scale = gfc_vector2d(2, 2);
	self->free = player_free;
	thePlayer = self;
	return self;
}