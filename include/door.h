#ifndef __DOOR_H__
#define __DOOR_H__

#include "entity.h"

/**
* @brief spawn a new door
* @param position the to spawn the door
* @return NULL if error, pointer to new item otherwise
*/
Entity* door_new(GFC_Vector2D position);

#endif