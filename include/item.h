#ifndef __ITEM_H__
#define __ITEM_H__

#include "entity.h"

/**
* @brief spawn a new item
* @param position the to spawn the item
* @param type the type of item to spawn
* @return NULL if error, pointer to new item otherwise
*/
Entity* item_new(GFC_Vector2D position, ItemTypes type);

#endif