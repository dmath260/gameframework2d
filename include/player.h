#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

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

typedef enum
{
	SO_NONE = 0,
	SO_Power1 = 1,
	SO_Power2 = 2,
	SO_PowerMax = 3,
	SO_Speed1 = 4,
	SO_Speed2 = 8,
	SO_SpeedMax = 12,
	SO_DoubleJump = 16,
	SO_Hover = 32,
	SO_MaxHealth = 64,
	SO_ALL = 127
}SkillsOwned;

typedef enum
{
	LB_NONE = 0,
	LB_Level1 = 1,
	LB_Level2 = 2,
	LB_Level3 = 4,
	LB_ALL = 7
}LevelsBeaten;

typedef struct
{
	PlayerStates state;
	Uint8 itemFlag;
	Uint8 skillPoints;
	SkillsOwned skills;
	LevelsBeaten levelsBeaten;
} PlayerData;

/**
* @brief get a pointer to the player if it exists
* @return a pointer to the player
*/
Entity* player_entity_get();

/**
* @brief create a new player entity
* @param position the spawn position of the player
* @return NULL if out of entities, a pointer to the newly-created player otherwise
*/
Entity* player_entity_new(GFC_Vector2D position);

/**
* @brief gives the player an item effect
* @param player a pointer to the player
* @param type the type of item to give the player
*/
void player_give_item(Entity *player, ItemTypes type);

/**
* @brief gives the player some skill points
* @param player a pointer to the player
* @param points how many points to give
*/
void give_skill_points(Entity* player, Uint8 points);

/**
* @brief takes some skill points from the player
* @param player a pointer to the player
* @param points how many points to take
*/
void spend_skill_points(Entity* player, Uint8 points);

/**
* @brief returns the number of skill points the player has
* @param player a pointer to the player
* @return the current number of skill points owned or 0 on error
*/
Uint8 get_skill_points(Entity* player);

/**
* @brief sets the player's skill points
* @param player a pointer to the player
* @param points how many points the player will have
*/
void set_skill_points(Entity* player, Uint8 points);

/**
* @brief checks if the player has a certain skill or skills
* @param player a pointer to the player
* @param skill the skill(s) to check
* @return 1 if the player has the specified skill(s), 0 otherwise
*/
Uint8 check_skill(Entity* player, SkillsOwned skill);

/**
* @brief gives the player some skills
* @param player a pointer to the player
* @param skill the skill(s) to give the player
*/
void grant_skill(Entity* player, SkillsOwned skill);

/**
* @brief makes the player lose some skills
* @param player a pointer to the player
* @param skill the skill(s) to take from the player
* @note for debugging purposes only
*/
void revoke_skill(Entity* player, SkillsOwned skill);

/**
* @brief sets the player's skills
* @param player a pointer to the player
* @param skill the skill(s) the player will have
*/
void set_skills(Entity* player, SkillsOwned skills);

/**
* @brief saves that the player beat a specific level
* @param player a pointer to the player
* @param level_beaten the level the player just cleared
* @return 1 if level cleared for the first time, 0 otherwise (including on error)
*/
Uint8 beat_level(Entity* player, Uint8 level_beaten);

/**
* @brief checks if a level has been cleared
* @param player a pointer to the player
* @param level the level(s) to check
* @return 1 if level already cleared, 0 otherwise (including on error)
*/
Uint8 level_cleared(Entity* player, Uint8 level);

/**
* @brief saves player data to a binary file (currently "saves/player.sav")
* @param player a pointer to the player
*/
void player_save(Entity* player);

/**
* @brief loads player data from a binary file (currently "saves/player.sav")
* @return NULL if file not found, loaded player data otherwise
*/
PlayerData* player_load();

/**
* @brief wipes the player data by setting everything to 0
*/
void wipe_data();

/**
* @brief updates and draws the HUD
* @param player a pointer to the player
* @note consider moving elsewhere when this becomes more important
*/
void hud_update(Entity* player);

/**
* @brief kills the player and slogs a death message
*/
void player_kill();

#endif