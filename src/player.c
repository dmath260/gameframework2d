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
	Uint8 itemFlag;
} PlayerData;

static Entity* thePlayer = NULL;

Entity* player_entity_get()
{
	return thePlayer;
}

PlayerData* get_data(Entity* self)
{
	if ((!self) || (!self->data)) return NULL;
	return (PlayerData*)self->data;
}

void player_free(Entity* self)
{
	PlayerData* data;
	data = get_data(self);
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
	data = get_data(self);
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
	data = get_data(self);
	if (data->state == state) return;
	data->state = state;
	self->data = data;
	entity_load(self, player_state_to_str(self));
}

void player_give_item(Entity* player, ItemTypes type)
{
	PlayerData* data;
	if (!player || !player->data) return;
	data = get_data(player);

	if (player->itemFrames)
	{
		if (player->item == type) slog("Resetting powerup duration");
		else if (player->item == IT_Invincible)
		{
			slog("Cannot override invincibility with another powerup");
			return;
		}
		else slog("Replacing powerup");
	}

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
			player->iFrames = player->itemFrames;
			player->color = gfc_color_hsl(0, 1, 0.5, 1);
			break;
		default: player->item = IT_NONE;
	}

	data->itemFlag = 0;
	player->data = data;
}

void player_entity_think(Entity* self)
{
	PlayerData* data;
	if (!self || !self->data) return;
	data = get_data(self);

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
		data->itemFlag = 0;
	}
	else if (gfc_input_key_down("w") && self->isGrounded)
	{
		self->velocity.y += self->impulse;
		self->isGrounded = 0;
		if (self->item == IT_DoubleJump) data->itemFlag = 1;
		else data->itemFlag = 0;
	}
	else if (self->isClimbing)
	{
		self->thinkPos.y = self->position.y;
		data->itemFlag = 0;
	}
	else if (gfc_input_key_pressed("w") && self->item == IT_DoubleJump && data->itemFlag == 1)
	{
		data->itemFlag = 0;
		self->velocity.y = self->impulse;
	}
	else if (gfc_input_key_down("w") && self->item == IT_Hover && self->velocity.y >= 0 && self->velocity.y < 0.25)
	{
		if (!data->itemFlag) data->itemFlag = 60;
		else data->itemFlag--;
		if (data->itemFlag)
		{
			self->velocity.y = -0.25; // velocity will be 0 on next frame as well
		}
	}
	else if (self->isGrounded) data->itemFlag = 0;
	if ((gfc_input_key_down("LSHIFT") || gfc_input_key_down("RSHIFT")) && self->isGrounded && move.x)
	{
		// Has to be grounded because sprinting in midair makes no sense
		// Also makes no sense to start sprinting before moving (matters when jumping)
		self->speedMult = 2.0 * (1 + (self->item == IT_Speed));
	}
	else if (!gfc_input_key_down("LSHIFT") && !gfc_input_key_down("RSHIFT") || !move.x)
	{
		// However, if the player was sprinting before jumping, don't kill their horizontal momentum
		// If the player stops moving horizontally, they have no momentum, so reduce their speed
		self->speedMult = 1.0 * (1 + (self->item == IT_Speed));
	}
	if (gfc_input_key_pressed(" "))
	{
		bullet_new(
			gfc_vector2d(self->position.x + 25, self->position.y),
			gfc_color8(0, 204, 255, 255),
			self->team,
			(self->animationData->FrameRow - 2) / 4,
			self->attack * (1 + (self->item == IT_Power))
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
	
	self->data = data;
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
	if (self->team == other->team || other->team == 2) return 1;
	if (self->item == IT_Invincible && other->maxHealth <= 20)
	{
		entity_hurt(other, other->maxHealth);
	}
	else if (!self->iFrames) {
		entity_hurt(self, other->attack);
		if (other->attack == 255u) slog("THE IMMORTAL SNAIL FOUND YOU");
	}
	return 1;
}

Entity* player_entity_new(GFC_Vector2D position)
{
	Entity* self;
	self = player_entity_get();
	if (self)
	{
		self->position = position;
		self->thinkPos = position;
		return self;
	}

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