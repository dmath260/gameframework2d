#include <stdio.h>

#include "simple_logger.h"

#include "gf2d_sprite.h"
#include "gf2d_graphics.h"

#include "door.h"
#include "audio.h"
#include "level.h"
#include "camera.h"
#include "boss1.h"
#include "monster.h"
#include "player.h"

void level_setup_camera_bounds(Level* level);
void level_load_entities(Level* level);

Level* current_level;

Level* level_new()
{
	Level* level;
	level = gfc_allocate_array(sizeof(Level), 1);
	if (!level) return NULL;
	return level;
}

Level* level_load(const char* filepath)
{
	int i, j, index;
	const char* str;
	Level* level;
	SJson* json;
	SJson* config;
	SJson *rows, *tiles, *tile;
	GFC_TextLine buffer;

	if (!filepath)
	{
		slog("JSON filepath invalid");
		return NULL;
	}

	strncpy(buffer, filepath, sizeof(GFC_TextLine) - 1);
	buffer[sizeof(buffer) - 1] = '\0';

	if (get_current_level())
	{
		level = get_current_level();
		set_current_level(NULL);
		level_free(level);
	}
	slog("%s", buffer);
	json = sj_load(buffer);
	if (!json)
	{
		slog("JSON data invalid");
		return NULL;
	}

	config = sj_object_get_value(json, "level");
	if (!config)
	{
		slog("failed to load level file %s, missing level information", filepath);
		sj_free(json);
		return NULL;
	}

	level = level_new();
	if (!level)
	{
		slog("failed to allocate level data for level %s", filepath);
		sj_free(json);
		return NULL;
	}

	str = sj_object_get_string(config, "background");
	if (str)
	{
		level->background = gf2d_sprite_load_image(str);
	}

	level->music_intro = _strdup(sj_object_get_string(config, "music_intro"));
	level->music_loop = _strdup(sj_object_get_string(config, "music_loop"));
	load_level_music(level);

	level->tileDef = tiledef_parse(sj_object_get_value(config, "tileDef"));
	if (!level->tileDef)
	{
		level_free(level);
		sj_free(json);
		slog("failed to parse tiledef data from level %s", filepath);
		return NULL;
	}

	rows = sj_object_get_value(config, "tileMap");
	level->height = sj_array_count(rows);
	if (!level->height)
	{
		level_free(level);
		sj_free(json);
		slog("undefined tilemap in level %s (no height)", filepath);
		return NULL;
	}

	tiles = sj_array_nth(rows, 0);
	level->width = sj_array_count(tiles);
	if (!level->width)
	{
		level_free(level);
		sj_free(json);
		slog("undefined tilemap in level %s (no width)", filepath);
		return NULL;
	}

	level->tileMap = gfc_allocate_array(sizeof(Uint8), level->width * level->height);
	if (!level->tileMap)
	{
		level_free(level);
		sj_free(json);
		slog("undefined tilemap in level %s (no tilemap)", filepath);
		return NULL;
	}

	for (j = 0; j < level->height; j++)
	{
		tiles = sj_array_get_nth(rows, j);
		if (!tiles) continue;
		for (i = 0; i < level->width; i++)
		{
			tile = sj_array_get_nth(tiles, i);
			if (!tile) continue;
			index = level_get_tile_index(level, i, j);
			if (index == -1) continue;
			sj_get_uint8_value(tile, &level->tileMap[index]);
		}
	}

	rows = sj_object_get_value(config, "entityMap");
	level->height = sj_array_count(rows);
	if (!level->height)
	{
		level_free(level);
		sj_free(json);
		slog("undefined entity map in level %s (no height)", filepath);
		return NULL;
	}

	tiles = sj_array_nth(rows, 0);
	level->width = sj_array_count(tiles);
	if (!level->width)
	{
		level_free(level);
		sj_free(json);
		slog("undefined entity map in level %s (no width)", filepath);
		return NULL;
	}

	level->entityMap = gfc_allocate_array(sizeof(Uint8), level->width * level->height);
	if (!level->entityMap)
	{
		level_free(level);
		sj_free(json);
		slog("undefined entity map in level %s (no entity map)", filepath);
		return NULL;
	}

	for (j = 0; j < level->height; j++)
	{
		tiles = sj_array_get_nth(rows, j);
		if (!tiles) continue;
		for (i = 0; i < level->width; i++)
		{
			tile = sj_array_get_nth(tiles, i);
			if (!tile) continue;
			index = level_get_tile_index(level, i, j);
			if (index == -1) continue;
			sj_get_uint8_value(tile, &level->entityMap[index]);
		}
	}

	str = sj_object_get_string(config, "nextLevel");
	if (str)
	{
		level->nextLevel = _strdup(str);
	}
	sj_object_get_uint8(config, "nextIsJSON", &level->nextIsJSON);

	level_bake_tiles(level);
	level_load_entities(level);
	level_setup_camera_bounds(level);
	
	sj_free(json);
	set_current_level(level);
	return level;
}

Level* get_current_level()
{
	return current_level;
}

void set_current_level(Level* level)
{
	current_level = level;
}

int level_get_tile_index(Level *level, Uint32 x, Uint32 y)
{
	if (!level || !level->tileMap) return -1;
	if (x >= level->width) return -1;
	if (y >= level->height) return -1;
	return y * level->width + x;
}

void level_add_border(Level* level, Uint8 tile) {
	Uint32 i, j, index;
	if (!level || !level->tileMap) return;
	for (j = 0; j < level->height; j++)
	{
		index = level_get_tile_index(level, 0, j);
		if (index >= 0) level->tileMap[index] = tile;
		index = level_get_tile_index(level, level->width - 1, j);
		if (index >= 0) level->tileMap[index] = tile;
	}
	for (i = 1; i < level->width - 1; i++)
	{
		index = level_get_tile_index(level, i, 0);
		if (index >= 0) level->tileMap[index] = tile;
		index = level_get_tile_index(level, i, level->height - 1);
		if (index >= 0) level->tileMap[index] = tile;
	}
}

void level_free(Level* level)
{
	if (!level) return;
	gf2d_sprite_free(level->background);
	gf2d_sprite_free(level->tileLayer);
	gf2d_sprite_free(level->tileDef->sheet);
	if (level->tileMap) free(level->tileMap);
	free(level->nextLevel);
	entity_manager_free_all_but_player();
	entity_manager_free_all_but_player(); // called a second time to delete any powerups that dropped
	if (level = get_current_level()) set_current_level(NULL);
	free(level);
}

void level_setup_camera_bounds(Level* level)
{
	if (!level) return;
	camera_set_bounds(gfc_rect(0, 0, level->tileDef->width * level->width, level->tileDef->height * level->height));
}

void level_bake_tiles(Level* level)
{
	SDL_Surface* tileSurface;
	Uint8 tile;
	Uint32 i, j;
	int index;
	if (!level) return;
	if (!level->width || !level->height) return;
	if (!level->tileDef->width || !level->tileDef->height) return;
	level->size.x = level->tileDef->width * level->width;
	level->size.y = level->tileDef->height * level->height;
	tileSurface = gf2d_graphics_create_surface(level->size.x, level->size.y);
	if (!tileSurface)
	{
		slog("failed to allocate a surface for the size of the image requested");
		return;
	}
	for (j = 0; j < level->height; j++)
	{
		for (i = 0; i < level->width; i++)
		{
			index = level_get_tile_index(level, i, j);
			if (index < 0) continue;
			tile = level->tileMap[index];
			if (!tile) continue;
			gf2d_sprite_draw_to_surface(
				level->tileDef->sheet,
				gfc_vector2d((i * level->tileDef->width), (j * level->tileDef->height)),
				NULL,
				NULL,
				tile - 1,
				tileSurface
			);
		}
	}
	if (level->tileLayer) { // free old layer before assigning new layer
		gf2d_sprite_free(level->tileLayer);
		level->tileLayer = NULL;
	}
	level->tileLayer = gf2d_sprite_from_surface(
		tileSurface,
		-1,
		-1,
		0,
		0
	);
}

void level_load_entities(Level* level)
{
	int i, j, index, x, y;
	Uint8 entId;
	if (!level) return;
	if (!level->width || !level->height) return;
	if (!level->tileDef->width || !level->tileDef->height) return;

	player_entity_new(gfc_vector2d(0, 0));

	for (j = 0; j < level->height; j++)
	{
		for (i = 0; i < level->width; i++)
		{
			index = level_get_tile_index(level, i, j);
			if (index < 0) continue;
			entId = level->entityMap[index];
			if (!entId) continue;
			x = i * level->tileDef->width;
			y = j * level->tileDef->height;

			/*
			* Entity ID cheat sheet:
			* 000: no entity
			* 001: player
			* 002-008: undefined
			* 009: door
			* 01X: monster_grunt
			* 02X: monster_seeker
			* 03X: monster_gunner
			* 04X: monster_flier
			* 05X: monster_immortalsnail
			* 060: boss1 (TBD)
			* 070: boss2 (TBD)
			* 080: boss3 (TBD)
			* 061-069, 071-079, 081-255: undefined
			* 
			* Ending digits (replacing X):
			* 0: no item on defeat
			* 1: power boost on defeat
			* 2: speed boost on defeat
			* 3: double jump on defeat
			* 4: hover on defeat
			* 5: invincibility on defeat
			* 6-9: undefined
			*/

			if (entId == 1) {
				player_entity_new(gfc_vector2d(x, y));
				slog("Player spawn coords: %i %i", x, y);
			}
			else if (entId == 9) door_new(gfc_vector2d(x, y));
			else if (entId >= 10 && entId < 60)
			{
				monster_new(
					gfc_vector2d(x, y),
					(MonsterTypes)(entId / 10 - 1),
					(ItemTypes)(entId % 10)
				);
			}
			else if (entId >= 60 && entId < 90)
			{
				if (entId == 60) boss1_new(gfc_vector2d(x, y));
				// add boss2 and boss3 for 70 and 80 when applicable
			}
		}
	}

	Entity* player = player_entity_get();
	camera_center_on(player->position);
}

void level_save_bin(Level* level, const char* filename)
{
	FILE *file;
	GFC_TextLine blank = {0};
	if (!level || !filename) return;
	file = fopen(filename, "wb");
	if (!file)
	{
		slog("Failed to save level to file %s", filename);
		return;
	}

	// update this every time you update the level structure
	if (level->background)
	{
		fwrite(level->background->filepath, sizeof(GFC_TextLine), 1, file);
	}
	else
	{
		fwrite(blank, sizeof(GFC_TextLine), 1, file);
	}

	if (level->music_intro)
	{
		fwrite(level->music_intro, sizeof(GFC_TextLine), 1, file);
	}
	else
	{
		fwrite(blank, sizeof(GFC_TextLine), 1, file);
	}

	if (level->music_loop)
	{
		fwrite(level->music_loop, sizeof(GFC_TextLine), 1, file);
	}
	else
	{
		fwrite(blank, sizeof(GFC_TextLine), 1, file);
	}

	fwrite(&level->width, sizeof(Uint32), 1, file);
	fwrite(&level->height, sizeof(Uint32), 1, file);
	fwrite(level->tileMap, sizeof(Uint8), level->width * level->height, file);
	fwrite(level->entityMap, sizeof(Uint8), level->width * level->height, file);

	if (level->nextLevel)
	{
		fwrite(level->nextLevel, sizeof(GFC_TextLine), 1, file);
	}
	else
	{
		fwrite(blank, sizeof(GFC_TextLine), 1, file);
	}
	fwrite(&level->nextIsJSON, sizeof(Uint8), 1, file);

	tiledef_save_to_file(level->tileDef, file);
	fclose(file);
}

Level *level_load_bin(const char* filename)
{
	static GFC_TextLine buffer = {0};
	FILE* file;
	if (!filename) return NULL;
	Level* level;
	if (filename) {
		strncpy(buffer, filename, sizeof(GFC_TextLine) - 1);
		buffer[sizeof(buffer) - 1] = '\0';
	}

	if (get_current_level())
	{
		level = get_current_level();
		set_current_level(NULL);
		level_free(level);
	}

	file = fopen(buffer, "rb");
	if (!file)
	{
		slog("Failed to open level file %s", buffer);
		return NULL;
	}

	// update this every time you update the level structure
	level = level_new();
	if (!level)
	{
		fclose(file);
		slog("Failed to allocate a new level");
		return NULL;
	}

	fread(buffer, sizeof(GFC_TextLine), 1, file);
	level->background = gf2d_sprite_load_image(buffer);
	fread(buffer, sizeof(GFC_TextLine), 1, file);
	level->music_intro = _strdup(buffer);
	fread(buffer, sizeof(GFC_TextLine), 1, file);
	level->music_loop = _strdup(buffer);
	slog("%s\n%s", level->music_intro, level->music_loop);
	load_level_music(level);

	fread(&level->width, sizeof(Uint32), 1, file);
	fread(&level->height, sizeof(Uint32), 1, file);
	if (!level->width || !level->height)
	{
		slog("Level file %s is bad: width: %i, height: %i", filename, level->width, level->height);
		level_free(level);
		fclose(file);
		return NULL;
	}

	level->tileMap = gfc_allocate_array(sizeof(Uint8), level->width * level->height);
	if (!level->tileMap)
	{
		slog("Failed to allocate tilemap for %s: width: %i, height: %i", filename, level->width, level->height);
		level_free(level);
		fclose(file);
		return NULL;
	}

	level->entityMap = gfc_allocate_array(sizeof(Uint8), level->width * level->height);
	if (!level->entityMap)
	{
		slog("Failed to allocate entity map for %s: width: %i, height: %i", filename, level->width, level->height);
		level_free(level);
		fclose(file);
		return NULL;
	}

	Uint32 total = level->width * level->height;
	fread(level->tileMap, sizeof(Uint8), total, file);
	fread(level->entityMap, sizeof(Uint8), total, file);
	fread(buffer, sizeof(GFC_TextLine), 1, file);
	level->nextLevel = _strdup(buffer);
	fread(&level->nextIsJSON, sizeof(Uint8), 1, file);
	level->tileDef = tiledef_load_from_file(file);

	level_bake_tiles(level);
	level_load_entities(level);
	level_setup_camera_bounds(level);

	fclose(file);
	set_current_level(level);
	return level;
}

/*
WIP function

void level_slog(Level* level)
{
	if (!level) return;
	if (level->background) slog("Level background: %s", level->background->filepath);
	slog("Level tile width: %i, height %i", level->width, level->height);
}
*/

void level_draw(Level* level)
{
	GFC_Vector2D offset;
	if (!level) return;
	offset = camera_get_offset();
	if (level->background) gf2d_sprite_draw_image(level->background, gfc_vector2d(0, 0));
	if (level->tileLayer) gf2d_sprite_draw_image(level->tileLayer, offset);
}