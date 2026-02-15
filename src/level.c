#include "simple_logger.h"

#include "gf2d_sprite.h"

#include "level.h"
#include "camera.h"

void level_setup_camera_bounds(Level* level);

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
	gf2d_sprite_free(level->tileDef->sheet);
	if (level->tileMap) free(level->tileMap);
	free(level);
}

void level_setup_camera_bounds(Level* level)
{
	if (!level) return;
	camera_set_bounds(gfc_rect(0, 0, level->tileDef->width * level->width, level->tileDef->height * level->height));
}

void level_draw(Level* level)
{
	GFC_Vector2D offset;
	Uint8 tile;
	Uint32 i, j, index;
	if (!level) return;
	offset = camera_get_offset();
	if (level->background)
	{
		gf2d_sprite_draw_image(level->background, offset);
	}
	if (level->tileDef->sheet) {
		for (j = 0; j < level->height; j++)
		{
			for (i = 0; i < level->width; i++)
			{
				index = level_get_tile_index(level, i, j);
				if (index < 0) continue;
				tile = level->tileMap[index];
				if (!tile) continue;
				gf2d_sprite_draw(
					level->tileDef->sheet,
					gfc_vector2d((i * level->tileDef->width) + offset.x, (j * level->tileDef->height) + offset.y),
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					tile - 1
				);
			}
		}
	}
}