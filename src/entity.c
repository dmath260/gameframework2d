#include "simple_logger.h"

#include "gf2d_draw.h"

#include "entity.h"
#include "camera.h"
#include "level.h"
#include "player.h"

typedef struct
{
	Entity	*entityList;
	Uint32	entityMax;
	Uint32	entityPool;
}EntityManager;

static EntityManager entityManager = {0};

void entity_manager_close(void);

void entity_manager_init(Uint32 max)
{
	if (!max)
	{
		slog("cannot initialize entity system with zero entities");
		return;
	}
	entityManager.entityList = gfc_allocate_array(sizeof(Entity), max);
	if (!entityManager.entityList)
	{
		slog("failed to allocate %i entities", max);
		return;
	}
	entityManager.entityMax = max;
	atexit(entity_manager_close);
	slog("initialized entity system");
}

void entity_manager_close(void)
{
	Uint32 i;
	if (!entityManager.entityList) return;
	for (i = 0; i < entityManager.entityMax; i++)
	{
		entity_free(&entityManager.entityList[i]);
	}
	free(entityManager.entityList);
	memset(&entityManager, 0, sizeof(EntityManager));
	slog("closed entity system");
}

Entity* entity_new()
{
	Uint32 i;
	if (!entityManager.entityList) {
		slog("entity system has not been initialized");
		return NULL;
	}
	for (i = 0; i < entityManager.entityMax; i++)
	{
		if (entityManager.entityList[i]._inuse) continue;
		entityManager.entityList[i]._inuse = true;
		entityManager.entityList[i].id = ++entityManager.entityPool;
		entityManager.entityList[i].color = GFC_COLOR_WHITE;
		//set defaults
		entityManager.entityList[i].scale.x = 1;
		entityManager.entityList[i].scale.y = 1;
		entityManager.entityList[i].gravity = 1;
		return &entityManager.entityList[i];
	}
	return NULL;
}

Entity* entity_get_by_id(Uint32 id)
{
	Uint32 i;
	if (!entityManager.entityList) {
		slog("entity system has not been initialized");
		return NULL;
	}
	for (i = 0; i < entityManager.entityMax; i++)
	{
		if (entityManager.entityList[i]._inuse) continue;
		if (entityManager.entityList[i].id == id) return &entityManager.entityList[i];
	}
	return NULL;
}

void entity_free(Entity* self)
{
	if (!self) return;
	if (self->free) self->free(self);
	if (self->animationData) {
		animdata_free(self->animationData);
		self->animationData = NULL;
	}
	memset(self, 0, sizeof(Entity));
}

void entity_draw(Entity *self)
{
	GFC_Rect bounds;
	GFC_Vector2D position, offset;
	if (!self) return;
	offset = camera_get_offset();
	gfc_vector2d_add(position, self->position, offset);
	gf2d_sprite_draw(
		self->animationData->Sprite,
		position,
		&self->scale,
		&self->rotationCenter,
		&self->rotation,
		NULL,
		&self->color,
		(Uint32)self->frame);
	bounds = self->bounds;
	gfc_vector2d_add(bounds, bounds, position);
	gf2d_draw_rect(bounds, GFC_COLOR_RED);
}

void entity_manager_draw_all()
{
	Uint32 i;
	if (!entityManager.entityList) {
		slog("entity system has not been initialized");
		return;
	}
	for (i = 0; i < entityManager.entityMax; i++)
	{
		if (!entityManager.entityList[i]._inuse) continue;
		entity_draw(&entityManager.entityList[i]);
	}
}

float binary_search_position(Entity *self, Uint8 axis, int start, int end)
{
	float start_pos, target_pos, mid, offset, lower, upper;
	int i, j, index, tw, th, coll;
	Level* current_level;

	current_level = get_current_level();
	if (!self || !current_level) return NAN;
	if (axis)
	{
		offset = (self->velocity.y > 0) ? self->bounds.h : 0;
		start_pos = self->position.y + offset;
		target_pos = self->thinkPos.y + offset;
	}
	else
	{
		offset = (self->velocity.x > 0) ? self->bounds.w : 0;
		start_pos = self->position.x + offset;
		target_pos = self->thinkPos.x + offset;
	}
	
	if (start_pos == target_pos) return (target_pos - offset);
	tw = current_level->tileDef->width;
	th = current_level->tileDef->height;

	lower = (start_pos < target_pos) ? start_pos : target_pos;
	upper = (start_pos < target_pos) ? target_pos : start_pos;

	for (i = 0; i < 20; i++)
	{
		mid = (upper + lower) / 2;
		coll = 0;
		for (j = start; j <= end; j++)
		{
			if (axis) index = level_get_tile_index(current_level, j * tw + (tw / 2), (Uint32)(mid / th));
			else index = level_get_tile_index(current_level, mid / tw, j * th + (th / 2));
			if (current_level->tileMap[index] > 0) {
				coll = 1;
				break;
			}
		}
		if (coll)
		{
			if (target_pos > start_pos) upper = mid;
			else lower = mid;
		}
		else
		{
			if (target_pos > start_pos) lower = mid;
			else upper = mid;
		}
	}

	mid = (upper + lower) / 2;
	if (target_pos > start_pos) mid -= 0.001;
	else mid += 0.001;
	return mid - offset;
}

Uint8 check_bounds(Entity* self, Uint8 axis)
{
	// axis: 0 for x, anything else for y
	if (!self) return 0;
	GFC_Rect bounds, indices;
	Level* current_level;
	int i, tw, th, x, y;

	bounds = self->bounds;
	gfc_vector2d_add(bounds, bounds, self->thinkPos);
	current_level = get_current_level();
	tw = current_level->tileDef->width;
	th = current_level->tileDef->height;
	indices = gfc_rect(
		(int)(bounds.x / tw),
		(int)(bounds.y / th),
		(int)((bounds.x + bounds.w - 1) / tw),
		(int)((bounds.y + bounds.h - 1) / th)
	);

	/*
	* Ignore the following special tiles:
	* 67 and 69: disabled switch blocks
	* 73: vines
	*/

	if (!axis)
	{
		// x-axis
		if (self->velocity.x < 0)
		{
			for (y = indices.y; y <= indices.h; y++)
			{
				i = level_get_tile_index(current_level, indices.x, y);
				if (current_level->tileMap[i] > 0)
				{
					//slog("A %i", current_level->tileMap[i]);
					if (current_level->tileMap[i] == 67) continue;
					if (current_level->tileMap[i] == 69) continue;
					if (current_level->tileMap[i] == 73) continue;
					return 1;
				}
			}
		}
		else if (self->velocity.x > 0)
		{
			for (y = indices.y; y <= indices.h; y++)
			{
				i = level_get_tile_index(current_level, indices.w, y);
				if (current_level->tileMap[i] > 0)
				{
					//slog("B %i", current_level->tileMap[i]);
					if (current_level->tileMap[i] == 67) continue;
					if (current_level->tileMap[i] == 69) continue;
					if (current_level->tileMap[i] == 73) continue;
					return 1;
				}
			}
		}
	}
	else
	{
		for (x = indices.x; x <= indices.w; x++)
		{
			if (self->velocity.y > 0)
			{
				i = level_get_tile_index(current_level, x, indices.h);
				if (i >= 0 && current_level->tileMap[i] > 0)
				{
					//slog("C %i", current_level->tileMap[i]);
					if (current_level->tileMap[i] == 67) continue;
					if (current_level->tileMap[i] == 69) continue;
					if (current_level->tileMap[i] == 73) continue;
					return 1;
				}
			}
			else if (self->velocity.y < 0)
			{
				i = level_get_tile_index(current_level, x, indices.y);
				if (current_level->tileMap[i] > 0)
				{
					//slog("D %i", current_level->tileMap[i]);
					if (current_level->tileMap[i] == 67) continue;
					if (current_level->tileMap[i] == 69) continue;
					if (current_level->tileMap[i] == 73) continue;
					return 1;
				}
			}
			else
			{
				return 1;
			}
		}
	}
	return 0;
}

void clip_to_bounds(Entity* self, Uint8 axis)
{
	// axis: 0 for x, anything else for y
	if (!self) return;
	GFC_Rect bounds, indices;
	Level* current_level;
	int i, j, k, tw, th, x, y;
	Uint8 switch_on;

	bounds = self->bounds;
	gfc_vector2d_add(bounds, bounds, self->thinkPos);
	current_level = get_current_level();
	tw = current_level->tileDef->width;
	th = current_level->tileDef->height;
	indices = gfc_rect(
		(int)(bounds.x / tw),
		(int)(bounds.y / th),
		(int)((bounds.x + bounds.w - 1) / tw),
		(int)((bounds.y + bounds.h - 1) / th)
	);

	/*
	* Assumptions for special tiles:
	* Spikes are index 65
	* Trampolines are index 66
	* Red switch blocks (on by default) are index 67 when off and index 68 when on
	* Blue switch blocks (off by default) are index 69 when off and index 70 when on
	* Switches (on by default) are index 71 when on and index 72 when off
	* Vines are index 73
	*/

	if (!axis)
	{
		// x-axis
		self->isClimbing = 0;

		if (self->velocity.x < 0)
		{
			for (y = indices.y; y <= indices.h; y++)
			{
				i = level_get_tile_index(current_level, indices.x, y);
				if (current_level->tileMap[i] > 0)
				{
					if (current_level->tileMap[i] == 67 || current_level->tileMap[i] == 69) continue;
					if (current_level->tileMap[i] == 73)
					{
						self->isClimbing = 1;
						continue;
					}
					self->isClimbing = 0;
					self->thinkPos.x = binary_search_position(self, axis, indices.y, indices.h);
					self->velocity.x = 0;
					break;
				}
			}
		}
		else if (self->velocity.x > 0)
		{
			for (y = indices.y; y <= indices.h; y++)
			{
				i = level_get_tile_index(current_level, indices.w, y);
				if (current_level->tileMap[i] > 0)
				{
					if (current_level->tileMap[i] == 67 || current_level->tileMap[i] == 69) continue;
					if (current_level->tileMap[i] == 73)
					{
						self->isClimbing = 1;
						continue;
					}
					self->isClimbing = 0;
					self->thinkPos.x = binary_search_position(self, axis, indices.y, indices.h);
					self->velocity.x = 0;
					break;
				}
			}
		}
	}
	else
	{
		// y-axis
		self->isGrounded = 0;
		self->isClimbing = 0;

		for (x = indices.x; x <= indices.w; x++)
		{
			if (self->velocity.y > 0)
			{
				i = level_get_tile_index(current_level, x, indices.h);
				if (i >= 0 && current_level->tileMap[i] > 0)
				{
					if (current_level->tileMap[i] == 67 || current_level->tileMap[i] == 69) continue;
					if (current_level->tileMap[i] == 73)
					{
						self->isClimbing = 1;
						continue;
					}
					self->isClimbing = 0;
					self->thinkPos.y = binary_search_position(self, axis, indices.x, indices.w);
					self->velocity.y = 0;
					self->isGrounded = 1;
					if (current_level->tileMap[i] == 65) entity_hurt(self, 1);
					if (current_level->tileMap[i] == 66)
					{
						if (!self->gravity) break;
						else if (!self->impulse) self->velocity.y = 5;
						else if (self == player_entity_get() && gfc_input_key_down("w"))
							self->velocity.y = self->impulse * sqrt(2);
						else self->velocity.y = self->impulse / sqrt(2);
						self->isGrounded = 0;
					}
					break;
				}
			}
			else if (self->velocity.y < 0)
			{
				i = level_get_tile_index(current_level, x, indices.y);
				if (current_level->tileMap[i] > 0)
				{
					if (current_level->tileMap[i] == 67 || current_level->tileMap[i] == 69) continue;
					if (current_level->tileMap[i] == 73)
					{
						self->isClimbing = 1;
						continue;
					}
					self->isClimbing = 0;
					self->thinkPos.y = binary_search_position(self, axis, indices.x, indices.w);
					self->velocity.y = 0;
					if (current_level->tileMap[i] == 65) entity_hurt(self, 1);
					if (current_level->tileMap[i] == 71 || current_level->tileMap[i] == 72)
					{
						if (current_level->tileMap[i] == 71) switch_on = 1;
						else switch_on = 0;
						for (j = 0; j < current_level->width; j++)
						{
							for (k = 0; k < current_level->height; k++)
							{
								i = level_get_tile_index(current_level, j, k);
								if (i < 0) continue;
								if (current_level->tileMap[i] < 67 || current_level->tileMap[i] > 72) continue;
								if (switch_on)
								{
									if (current_level->tileMap[i] == 68) current_level->tileMap[i] = 67;
									if (current_level->tileMap[i] == 69) current_level->tileMap[i] = 70;
									if (current_level->tileMap[i] == 71) current_level->tileMap[i] = 72;
								}
								else
								{
									if (current_level->tileMap[i] == 67) current_level->tileMap[i] = 68;
									if (current_level->tileMap[i] == 70) current_level->tileMap[i] = 69;
									if (current_level->tileMap[i] == 72) current_level->tileMap[i] = 71;
								}
							}
						}
						level_bake_tiles(current_level);
					}
					break;
				}
			}
		}
	}
	if (self->isClimbing) self->velocity.y = 0;
}

Uint8 entity_collision_test(Entity* self, Entity *other)
{
	GFC_Rect bounds1, bounds2;
	if (!self || !other || self == other) return 0;
	bounds1 = self->bounds;
	bounds2 = other->bounds;
	gfc_vector2d_add(bounds1, self->bounds, self->position);
	gfc_vector2d_add(bounds2, other->bounds, other->position);
	// now in same space frame of reference
	return gfc_rect_overlap(bounds1, bounds2);
}

Uint8 entity_collision_test_world(Entity* self)
{
	Uint32 i;
	Entity* other;
	if (!self) return 0;
	for (i = 0; i < entityManager.entityMax; i++)
	{
		if (!entityManager.entityList[i]._inuse) continue;
		if (entity_collision_test(self, &entityManager.entityList[i]))
		{
			other = &entityManager.entityList[i];
			if (self->touch) self->touch(self, other);
			//if (other->touch) other->touch(other, self);
			return 1;
		}
	}
	return 0;
}

void entity_think(Entity* self) {
	if (!self) return;

	//downward velocity (if entity is affected by gravity)
	if (self->gravity)
	{
		if (!self->isClimbing) self->velocity.y += 0.25;
		self->thinkPos.y += self->velocity.y;
		clip_to_bounds(self, 1);
	}

	if (self->think) self->think(self);

	self->thinkPos.x += self->velocity.x;
	clip_to_bounds(self, 0);

	// Animation stuff, might want to move this into the animation class
	if (!self->animationData) return;
	AnimData *data;
	data = self->animationData;
	data->FrameCount += 0.5;
	if (data->FrameCount >= data->AnimFrames[data->FrameCol + 1]) data->FrameCol++;
	if (data->FrameCol == data->FramesPerRow) {
		data->FrameCol = 0;
		data->FrameCount = 0;
	}
}

void entity_manager_think_all()
{
	Uint32 i;
	if (!entityManager.entityList) {
		slog("entity system has not been initialized");
		return;
	}
	for (i = 0; i < entityManager.entityMax; i++)
	{
		if (!entityManager.entityList[i]._inuse) continue;
		if (!entityManager.entityList[i].think) continue;
		entity_think(&entityManager.entityList[i]);
	}
}

void entity_update(Entity *self)
{
	Level* current_level;
	if (!self) return;

	self->position.x = self->thinkPos.x;
	self->position.y = self->thinkPos.y;

	if (self->velocity.x > GFC_EPSILON || self->velocity.x < GFC_EPSILON * -1)
	{
		self->velocity.x *= 0.5;
	}
	else self->velocity.x = 0;

	if (self->iFrames) {
		if (self->iFrames == 1) self->color = GFC_COLOR_WHITE;
		self->iFrames--;
	}
	
	// Animation stuff, might want to move this into the animation class
	if (!self->animationData) return;
	AnimData* data;
	data = self->animationData;
	self->frame = (float)(data->FrameRow * data->FramesPerRow + data->FrameCol);

	if (self->update) self->update(self);

	current_level = get_current_level();
	if (self->position.x < 0 || self->position.x > current_level->width * current_level->tileDef->width)
	{
		self->health = 0;
	}

	// Last thing: check if entity is still alive
	if (self->health <= 0) {
		if (self == player_entity_get()) player_kill("Player ran out of health.");
		else entity_free(self);
	}
}

void entity_manager_update_all()
{
	Uint32 i;
	if (!entityManager.entityList) {
		slog("entity system has not been initialized");
		return;
	}
	for (i = 0; i < entityManager.entityMax; i++)
	{
		if (!entityManager.entityList[i]._inuse) continue;
		if (!entityManager.entityList[i].update) continue;
		entity_update(&entityManager.entityList[i]);
	}
}

void entity_hurt(Entity* self, Uint8 damage)
{
	if (self->iFrames) return;
	self->health -= damage;
	if (self->health <= 0) return;
	self->iFrames = self->maxIFrames;
	self->color = GFC_COLOR_GREY; // replace this w/ pain animation later
	if (self == player_entity_get()) slog("Ouch! Current health: %i", self->health);
}

void entity_load(Entity* self, char* state)
{
	SJson* json, * config;
	AnimData* animationData;

	if (!self)
	{
		slog("Invalid entity");
		return;
	}
	if (!state)
	{
		slog("Invalid state");
		return;
	}
	if (!self->animDataFilePath)
	{
		slog("JSON filepath invalid");
		return;
	}
	json = sj_load(self->animDataFilePath);
	if (!json)
	{
		slog("JSON data invalid");
		return;
	}

	config = sj_object_get_value(json, "AnimData");
	if (!config)
	{
		slog("Failed to load animation data for %s", self->animDataFilePath);
		sj_free(json);
		return;
	}

	animationData = animdata_parse(config, state, self->animationData);
	if (!animationData)
	{
		slog("Failed to parse animation data for %s", self->animDataFilePath);
		sj_free(json);
		return;
	}

	self->animationData = animationData;

	sj_free(json);
	return;
}