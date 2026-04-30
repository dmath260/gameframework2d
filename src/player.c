#include "simple_logger.h"

#include "gfc_input.h"

#include "bullet.h"
#include "player.h"
#include "camera.h"

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
	thePlayer = NULL;
}

void player_kill()
{
	if (!thePlayer) return;
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

	if (type == IT_HealthRestore)
	{
		if (player->health == player->maxHealth)
		{
			return;
		}
		else if (player->health >= player->maxHealth - 2)
		{
			player->health = player->maxHealth;
			return;
		}
		player->health += 2;
		return;
	}

	if (player->itemFrames && player->item == IT_Invincible) return;

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

void give_skill_points(Entity* player, Uint8 points)
{
	if (!player || !player->data) return NULL;
	PlayerData* data;
	data = get_data(player);
	if (data->skillPoints + points < data->skillPoints)
	{
		return NULL;
	}
	data->skillPoints += points;
}

void spend_skill_points(Entity* player, Uint8 points)
{
	if (!player || !player->data) return NULL;
	PlayerData* data;
	data = get_data(player);
	if (data->skillPoints - points > data->skillPoints)
	{
		return NULL;
	}
	data->skillPoints -= points;
}

void set_skill_points(Entity* player, Uint8 points)
{
	if (!player || !player->data) return NULL;
	PlayerData* data;
	data = get_data(player);
	data->skillPoints = points;
}

Uint8 check_skill(Entity* player, SkillsOwned skill)
{
	if (!player || !player->data) return 0;
	PlayerData* data;
	data = get_data(player);
	return ((data->skills & skill) == skill);
}

void grant_skill(Entity* player, SkillsOwned skill)
{
	if (!player || !player->data) return NULL;
	PlayerData* data;
	data = get_data(player);
	if (skill & SO_MaxHealth && !check_skill(player, SO_MaxHealth))
	{
		player->health += player->maxHealth;
		player->maxHealth *= 2;
	}
	data->skills |= skill;
}

void revoke_skill(Entity* player, SkillsOwned skill)
{
	if (!player || !player->data) return NULL;
	PlayerData* data;
	data = get_data(player);
	if (skill & SO_MaxHealth && check_skill(player, SO_MaxHealth))
	{
		player->maxHealth /= 2;
		player->health -= player->maxHealth;
	}
	data->skills &= ~skill;
}

void set_skills(Entity* player, SkillsOwned skills)
{
	if (!player || !player->data) return NULL;
	PlayerData* data;
	data = get_data(player);
	if (skills & SO_MaxHealth && !check_skill(player, SO_MaxHealth))
	{
		player->health += player->maxHealth;
		player->maxHealth *= 2;
	}
	else if (!(skills & SO_MaxHealth) && check_skill(player, SO_MaxHealth))
	{
		player->maxHealth /= 2;
		player->health -= player->maxHealth;
	}
	data->skills = skills;
}

void player_save(Entity* player)
{
	PlayerData* data;
	FILE* file;
	if (!player || !player->data) return;
	file = fopen("save/player.sav", "wb");
	if (!file)
	{
		return;
	}
	data = get_data(player);

	// update this every time you update the player structure
	fwrite(&data->skillPoints, sizeof(Uint8), 1, file);
	fwrite(&data->skills, sizeof(SkillsOwned), 1, file);
	fwrite(&data->levelsBeaten, sizeof(Uint8), 1, file);

	fclose(file);
}

PlayerData* player_load()
{
	PlayerData* data;
	FILE* file;
	file = fopen("save/player.sav", "rb");
	if (!file)
	{
		return NULL;
	}

	data = gfc_allocate_array(sizeof(PlayerData), 1);
	fread(&data->skillPoints, sizeof(Uint8), 1, file);
	fread(&data->skills, sizeof(SkillsOwned), 1, file);
	fread(&data->levelsBeaten, sizeof(Uint8), 1, file);

	fclose(file);
	return data;
}

void wipe_data()
{
	FILE* file;
	Uint8 zero_8 = 0;
	SkillsOwned zero_32 = 0;
	file = fopen("save/player.sav", "wb");
	if (!file)
	{
		return;
	}

	// update this every time you update the player structure
	fwrite(&zero_8, sizeof(Uint8), 1, file);
	fwrite(&zero_32, sizeof(SkillsOwned), 1, file);
	fwrite(&zero_8, sizeof(Uint8), 1, file);

	fclose(file);
}

void hud_update(Entity* player)
{
	int i;
	Sprite* heart;
	if (!player) return;

	heart = gf2d_sprite_load_all("images/heart/heart.png", 36, 36, 1, 0);

	for (int i = 0; i < player->health; i++)
	{
		gf2d_sprite_draw_image(heart, gfc_vector2d(40 * i + 12, 12));
	}
}

void player_entity_think(Entity* self)
{
	PlayerData* data;
	if (!self || !self->data) return;
	data = get_data(self);

	GFC_Vector2D move = {0};
	if (!self) return;

	// Replace these with gfc_input_command_down if you can figure out how it works
	if (gfc_input_key_pressed("UP"))
	{
		if (check_skill(self, SO_ALL))
		{
			set_skills(self, SO_NONE);
		}
		else if (check_skill(self, SO_MaxHealth))
		{
			set_skills(self, SO_ALL);
		}
		else if (check_skill(self, SO_Hover))
		{
			set_skills(self, SO_MaxHealth);
		}
		else if (check_skill(self, SO_DoubleJump))
		{
			set_skills(self, SO_Hover);
		}
		else if (check_skill(self, SO_SpeedMax))
		{
			set_skills(self, SO_DoubleJump);
		}
		else if (check_skill(self, SO_Speed1))
		{
			set_skills(self, SO_SpeedMax);
		}
		else if (check_skill(self, SO_PowerMax))
		{
			set_skills(self, SO_Speed1);
		}
		else if (check_skill(self, SO_Power1))
		{
			set_skills(self, SO_PowerMax);
		}
		else if (check_skill(self, SO_NONE))
		{
			set_skills(self, SO_Power1);
		}
	}
	else if (gfc_input_key_pressed("DOWN"))
	{
		if (check_skill(self, SO_ALL))
		{
			set_skills(self, SO_MaxHealth);
		}
		else if (check_skill(self, SO_MaxHealth))
		{
			set_skills(self, SO_Hover);
		}
		else if (check_skill(self, SO_Hover))
		{
			set_skills(self, SO_DoubleJump);
		}
		else if (check_skill(self, SO_DoubleJump))
		{
			set_skills(self, SO_SpeedMax);
		}
		else if (check_skill(self, SO_SpeedMax))
		{
			set_skills(self, SO_Speed1);
		}
		else if (check_skill(self, SO_Speed1))
		{
			set_skills(self, SO_PowerMax);
		}
		else if (check_skill(self, SO_PowerMax))
		{
			set_skills(self, SO_Power1);
		}
		else if (check_skill(self, SO_Power1))
		{
			set_skills(self, SO_NONE);
		}
		else if (check_skill(self, SO_NONE))
		{
			set_skills(self, SO_ALL);
		}
	}
	
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
		if (self->item == IT_DoubleJump || (data->skills & SO_DoubleJump)) data->itemFlag = 1;
		else data->itemFlag = 0;
	}
	else if (self->isClimbing)
	{
		self->thinkPos.y = self->position.y;
		data->itemFlag = 0;
	}
	else if (gfc_input_key_pressed("w") && data->itemFlag == 1 &&
		(self->item == IT_DoubleJump || (data->skills & SO_DoubleJump)))
	{
		data->itemFlag = 0;
		self->velocity.y = self->impulse;
	}
	else if (gfc_input_key_down("w") && (self->item == IT_Hover || (data->skills & SO_Hover)) &&
		self->velocity.y >= 0 && self->velocity.y < 0.25)
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
		self->speedMult = 2.0 / 3.0 * (3 + (self->item == IT_Speed) +
			(data->skills & SO_Speed1) + (data->skills & SO_Speed2));
	}
	else if (!gfc_input_key_down("LSHIFT") && !gfc_input_key_down("RSHIFT") || !move.x)
	{
		// However, if the player was sprinting before jumping, don't kill their horizontal momentum
		// If the player stops moving horizontally, they have no momentum, so reduce their speed
		self->speedMult = 1.0 / 3.0 * (3 + (self->item == IT_Speed) +
			(data->skills & SO_Speed1) + (data->skills & SO_Speed2));
	}

	if (gfc_input_key_pressed(" "))
	{
		bullet_new(
			gfc_vector2d(self->position.x + 25, self->position.y),
			gfc_color8(0, 204, 255, 255),
			self->team,
			(self->animationData->FrameRow - 2) / 4,
			self->attack * (1 + (self->item == IT_Power) + (data->skills & SO_Power1) + (data->skills & SO_Power2))
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
		self->velocity = gfc_vector2d(0, 0);
		return self;
	}

	self = entity_new();
	if (!self) return NULL;
	PlayerData* data;
	data = player_load();
	if (!data)
	{
		data = gfc_allocate_array(sizeof(PlayerData), 1);
	}
	self->data = data;
	self->animDataFilePath = "images/0258/0258AnimData.json";
	set_player_state(self, PS_Idle);
	self->animationData->FrameRow = 2;
	self->rotationCenter = gfc_vector2d(12, 16);
	self->bounds = gfc_rect(-18, -30, 52, 52); // change these values later AND move to set_player_state
	self->impulse = -10;
	self->topSpeed = 3;
	self->speedMult = 1;
	self->maxHealth = 10 * (1 + check_skill(self, SO_MaxHealth));
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