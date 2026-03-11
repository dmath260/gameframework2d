#ifndef __TILEDEF_H__
#define __TILEDEF_H__

#include <stdio.h>

#include "simple_json.h"

#include "gf2d_sprite.h"

typedef struct
{
	Sprite* sheet;
	Uint32 width, height, fpl;
}TileDef;

/**
* @brief save tiledef info to a binary file
* @param tiledef the tiledef to save
* @param file path to the file to save the tiledef to
* @return 1 if successful, 0 otherwise
*/
int tiledef_save_to_file(TileDef* tiledef, FILE *file);

/**
* @brief load tiledef info from a binary file
* @param file path to the file to load the tiledef from
* @return NULL if error, pointer to loaded tiledef otherwise
*/
TileDef *tiledef_load_from_file(FILE* file);

/**
* @brief free a tiledef object
* @param tiledef the tiledef to free
*/
void tiledef_free(TileDef* tiledef);

/**
* @brief parse json into a filedef
* @param config the json to parse
* @return NULL if error, pointer to loaded tiledef otherwise
*/
TileDef* tiledef_parse(SJson* config);

#endif