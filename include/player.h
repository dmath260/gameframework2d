#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

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
* @brief updates and draws the HUD
* @param player a pointer to the player
* @note consider moving elsewhere when this becomes more important
*/
void hud_update(Entity* player);

/**
* @brief kills the player and slogs a death message
* @message the message to slog
*/
void player_kill(char *message);

#endif