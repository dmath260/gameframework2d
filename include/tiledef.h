#ifndef __TILEDEF_H__
#define __TILEDEF_H__

#include "simple_json.h"

#include "gf2d_sprite.h"

typedef struct
{
	Sprite* sheet;
	Uint32 width, height, fpl;
}TileDef;

/**
* @brief lorem ipsum
* @param tiledef lorem ipsum
*/
void tiledef_free(TileDef* tiledef);

/**
* @brief parse json into a filedef
* @param config lorem ipsum
* @return lorem ipsum
*/
TileDef* tiledef_parse(SJson* config);

#endif