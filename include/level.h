#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "gf2d_sprite.h"

typedef struct
{
	Sprite* background;				//background sprite
	Sprite* tileSet;				//sprite containing the tiles to draw with
	Uint8* tileMap;					//pointer to the tile map data
	Uint32 width, height;			//size of the tileMap
	Uint32 tileWidth, tileHeight;	//size of each tile
}Level;

/**
* @brief allocate a blank level
* @return the newly allocated level
*/
Level* level_new();

/**
* @brief allocate and build a level from the parameters
* @param background the filepath to the background image
* @param tileMap the filepath to the tile sprite sheet
* @param tileWidth how wide a tile is in pixels
* @param tileHeight how tall a tile is in pixels
* @param tilesPerLine how many tiles per line in the sprite sheet
* @param width how many tiles wide the map is
* @param width how many tiles tall the map is
* @return NULL on error or nonsensical parameters, a pointer to a level otherwise
* @note the tilemap still needs to be set
*/
Level* level_create(
	const char* background,
	const char* tileSet,
	Uint32 tileWidth,
	Uint32 tileHeight,
	Uint32 tilesPerLine,
	Uint32 width,
	Uint32 height);
/**
* @brief given a leve, get the index of the tileMap for a tile's coordinates
* @param level to check
* @param x the x coordinate
* @param y the y coordinate
* @return -1 if the level is bad or the coordinates are beyond the tilemap, the index otherwise
*/
int level_get_tile_index(Level* level, Uint32 x, Uint32 y);

/**
* @brief set the border tiles of the level to the given tile
* @param level the level to set
* @param tile the tileMap index to set it to
*/
void level_add_border(Level* level, Uint8 tile);

/**
* @brief free a level;
* @param level the level to free
*/
void level_free(Level* level);

/**
* @brief draw the current level
* @param the level to draw
*/
void level_draw(Level* level);

#endif