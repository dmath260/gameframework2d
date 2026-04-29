#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "tiledef.h"

typedef struct
{
	Sprite* background;				//background sprite
	const char* music_intro;		//path to music intro
	const char* music_loop;			//path to music loop
	Sprite* tileLayer;				//static tile layer for fast rendering
	Uint8* tileMap;					//pointer to the tile map data
	Uint8* entityMap;				//data for where entities spawn when loading a level
	Uint32 width, height;			//size of the tileMap and entityMap
	GFC_Vector2D size;				//size of the tileMap and entityMap in pixels
	const char* nextLevel;			//filepath of the next level to load
	TileDef* tileDef;
}Level;

/**
* @brief allocate a blank level
* @return the newly allocated level
*/
Level* level_new();

/**
* @brief load a level from JSON
* @param filepath the path to the JSON
* @param 1 to play music, 0 to not play music
* @return the newly allocated level (or a NULL pointer if allocation fails)
*/
Level* level_load(const char* filepath, Uint8 music);

/**
* @brief save current level to JSON
* @param level the level to save
* @param filename the path of the file to save the level to
*/
void level_save_bin(Level* level, const char* filename);

/**
* @brief get the current level
* @return a pointer to the current level
*/
Level* get_current_level();

/**
* @brief set the current level (used in files other than level.c and game.c)
* @param level the level to set as the current level
*/
void set_current_level(Level* level);

/**
* @brief given a level, get the index of the tileMap for a tile's coordinates
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
* @brief bake tiles into the level
* @param level the level to bake tiles into
*/
void level_bake_tiles(Level* level);

/**
* @brief free a level
* @param level the level to free
*/
void level_free(Level* level);

/**
* @brief save current level to binary file
* @param level the level to save
* @param filename the path of the file to save the level to
*/
void level_save_bin(Level* level, const char* filename);

/**
* @brief load level from binary file
* @param filename the path of the file to load the level from
* @param 1 to play music, 0 to not play music
* @return NULL if level is invalid, a pointer to the loaded level otherwise
*/
Level *level_load_bin(const char* filename, Uint8 music);

/**
* @brief draw the current level
* @param the level to draw
*/
void level_draw(Level* level);

#endif