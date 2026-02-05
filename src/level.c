#include "simple_logger.h"

#include "gf2d_sprite.h"

#include "level.h"
#include "camera.h"

Level* level_new()
{
	Level* level;
	level = gfc_allocate_array(sizeof(Level), 1);
	if (!level) return NULL;
	return level;
}

Level* level_create(
	const char* background,
	const char* tileSet,
	Uint32 tileWidth,
	Uint32 tileHeight,
	Uint32 tilesPerLine,
	Uint32 width,
	Uint32 height)
{
	if (!width || !height)
	{
		slog("cannot create a level with a zero dimension");
		return NULL;
	}
	Level* level;
	level = level_new();
	if (!level) return NULL;
	if (background)
	{
		level->background = gf2d_sprite_load_image(background);
	}
	if (tileSet)
	{
		level->tileSet = gf2d_sprite_load_all(
			tileSet,
			tileWidth,
			tileHeight,
			tilesPerLine,
			1);
	}
	level->tileMap = gfc_allocate_array(sizeof(Uint8), width * height);
	level->width = width;
	level->height = height;
	level->tileWidth = tileWidth;
	level->tileHeight = tileHeight;

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
	gf2d_sprite_free(level->tileSet);
	if (level->tileMap) free(level->tileMap);
	free(level);
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
	if (level->tileSet) {
		for (j = 0; j < level->height; j++)
		{
			for (i = 0; i < level->width; i++)
			{
				index = level_get_tile_index(level, i, j);
				if (index < 0) continue;
				tile = level->tileMap[index];
				if (!tile) continue;
				gf2d_sprite_draw(
					level->tileSet,
					gfc_vector2d((i * level->tileWidth) + offset.x, (j * level->tileHeight) + offset.y),
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