#include <stdio.h>

#include "simple_logger.h"

#include "gf2d_sprite.h"
#include "gf2d_graphics.h"

#include "level.h"
#include "camera.h"

void level_setup_camera_bounds(Level* level);

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
	if (!filepath)
	{
		slog("JSON filepath invalid");
		return NULL;
	}
	json = sj_load(filepath);
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

	level_setup_camera_bounds(level);

	sj_free(json);
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
	//SDL_FreeSurface(tileSurface); Not necessary, the surface is freed in gf2d_sprite_from_surface
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

	fwrite(&level->width, sizeof(Uint32), 1, file);
	fwrite(&level->height, sizeof(Uint32), 1, file);
	fwrite(level->tileMap, sizeof(Uint8), level->width * level->height, file);
	tiledef_save_to_file(level->tileDef, file);
	fclose(file);
}

Level *level_load_bin(const char* filename)
{
	static GFC_TextLine buffer = {0};
	FILE* file;
	if (!filename) return NULL;
	Level* level;
	file = fopen(filename, "rb");
	if (!file)
	{
		slog("Failed to open level file %s", filename);
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
	// somehow everything corrupts when loading the sprite
	level->background = gf2d_sprite_load_image(buffer);
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

	Uint32 total = level->width * level->height;
	fread(level->tileMap, sizeof(Uint8), total, file);
	level->tileDef = tiledef_load_from_file(file);
	level_setup_camera_bounds(level);

	fclose(file);
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
	if (level->background) gf2d_sprite_draw_image(level->background, offset);
	if (level->tileLayer) gf2d_sprite_draw_image(level->tileLayer, offset);
}