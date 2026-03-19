#include "simple_logger.h"

#include "gfc_input.h"

#include "bullet.h"
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

void player_kill(char* message)
{
	if (!thePlayer) return;
	slog("%s", message);
	entity_free(thePlayer);
}

char* player_state_to_str(Entity* self)
{
	if (!self || !self->data) return "Idle";
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
	entity_load(self, player_state_to_str(self));
}

void player_give_item(Entity* player, ItemTypes type)
{
	if (!player) return;
	player->item = type;

	// frames should be ~90 * # of seconds
	switch (type)
	{
		case IT_Power:
			player->itemFrames = 90 * 60;
			break;
		case IT_Speed:
			player->itemFrames = 90 * 60;
			break;
		case IT_DoubleJump:
			player->itemFrames = 90 * 60;
			break;
		case IT_Hover:
			player->itemFrames = 90 * 60;
			break;
		case IT_Invincible:
			player->itemFrames = 90 * 10;
			player->color = gfc_color_hsl(0, 1, 0.5, 1);
			break;
		default: player->item = IT_NONE;
	}
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
	if (gfc_input_key_down("w") && self->isClimbing)
	{
		self->thinkPos.y -= self->topSpeed;
		self->velocity.y = self->topSpeed * -1;
		if (check_bounds(self, 1)) self->thinkPos.y = self->position.y;
		else self->velocity.y = 0;
	}
	else if (gfc_input_key_down("w") && self->isGrounded)
	{
		self->velocity.y += self->impulse;
		self->isGrounded = 0;
	}
	else if (self->isClimbing) self->thinkPos.y = self->position.y;
	if ((gfc_input_key_down("LSHIFT") || gfc_input_key_down("RSHIFT")) && self->isGrounded && move.x)
	{
		// Has to be grounded because sprinting in midair makes no sense
		// Also makes no sense to start sprinting before moving (matters when jumping)
		self->speedMult = 2.0;
	}
	else if (!gfc_input_key_down("LSHIFT") && !gfc_input_key_down("RSHIFT") || !move.x)
	{
		// However, if the player was sprinting before jumping, don't kill their horizontal momentum
		// If the player stops moving horizontally, they have no momentum, so reduce their speed
		self->speedMult = 1.0;
	}
	if (gfc_input_key_pressed(" "))
	{
		bullet_new(
			gfc_vector2d(self->position.x + 25, self->position.y),
			gfc_color8(0, 204, 255, 255),
			self->team,
			(self->animationData->FrameRow - 2) / 4,
			self->attack
		);
	}
	if (!self->isGrounded && self->velocity.y) set_player_state(self, PS_Jump);
	if (self->isClimbing) set_player_state(self, PS_Idle);
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
	entity_collision_test_world(self);
}

Uint8 player_entity_touch(Entity* self, Entity* other)
{
	if (!self || !other) return 0;
	if (self->iFrames || self->team == other->team || other->team == 2) return 1;
	entity_hurt(self, other->attack);
	if (other->attack == 255u) slog("THE IMMORTAL SNAIL FOUND YOU");
	return 1;
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
	self->animationData->FrameRow = 2;
	self->rotationCenter = gfc_vector2d(12, 16);
	self->bounds = gfc_rect(-18, -30, 52, 52); // change these values later AND move to set_player_state
	self->impulse = -10;
	self->topSpeed = 3;
	self->speedMult = 1;
	self->maxHealth = 10;
	self->health = self->maxHealth;
	self->attack = 2;
	self->maxIFrames = 90;
	self->team = 0; // team 0 for player
	self->position = position;
	self->thinkPos = position;
	self->think = player_entity_think;
	self->update = player_entity_update;
	self->touch = player_entity_touch;
	self->scale = gfc_vector2d(2, 2);
	self->free = player_free;
	thePlayer = self;
	return self;
}