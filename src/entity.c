#include "simple_logger.h"

#include "gf2d_draw.h"

#include "entity.h"
#include "camera.h"
#include "level.h"

typedef struct
{
	Entity	*entityList;
	Uint32	entityMax;
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
		//set defaults
		entityManager.entityList[i].scale.x = 1;
		entityManager.entityList[i].scale.y = 1;
		return &entityManager.entityList[i];
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
		NULL,
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

Uint8 entity_collision_test(Entity* self, Entity *other)
{
	GFC_Rect bounds1, bounds2;
	if (!self || !other || self == other) return;
	bounds1 = self->bounds;
	bounds2 = other->bounds;
	gfc_vector2d_add(bounds1, self->bounds, self->position);
	gfc_vector2d_add(bounds2, other->bounds, other->position);
	// now in same space frame of reference
	return gfc_rect_overlap(bounds1, bounds2);
}

Uint8 entity_collision_test_world(Entity* self)
{
	int i;
	if (!self) return;
	for (i = 0; i < entityManager.entityMax; i++)
	{
		if (!entityManager.entityList[i]._inuse) continue;
		if (entity_collision_test(self, &entityManager.entityList[i]))
		{
			if (self->touch) self->touch(self, &entityManager.entityList[i]);
			return 1;
		}
	}
	return 0;
}

void entity_think(Entity* self) {
	if (!self) return;

	//downward velocity
	self->velocity.y += 0.25;

	if (self->think) self->think(self);

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

void check_bounds(Entity *self, Uint8 axis)
{
	// axis: 0 for x, anything else for y
	if (!self) return;
	GFC_Rect bounds, indices;
	Level* current_level;
	int i, tw, th;

	bounds = self->bounds;
	gfc_vector2d_add(bounds, bounds, self->position);
	current_level = get_current_level();
	tw = current_level->tileDef->width;
	th = current_level->tileDef->height;
	indices = gfc_rect(
		(int)(bounds.x / tw),
		(int)(bounds.y / th),
		(int)((bounds.x + bounds.w - 1) / tw),
		(int)((bounds.y + bounds.h - 1) / th)
	);

	i = level_get_tile_index(current_level, indices.x, indices.y);
	if (current_level->tileMap[i] > 1) {
		if (!axis)
		{
			self->position.x = tw * (indices.x + 1) - self->bounds.x;
			self->velocity.x = 0;
		}
		else
		{
			self->position.y = th * (indices.y + 1) - self->bounds.y;
			self->velocity.y = 0;
		}
	}
	if (!axis)
	{
		i = level_get_tile_index(current_level, indices.w, indices.y);
		if (current_level->tileMap[i] > 1) {
			self->position.x = tw * (indices.w) - (self->bounds.x + self->bounds.w);
			self->velocity.x = 0;
		}
	}
	else
	{
		i = level_get_tile_index(current_level, indices.x, indices.h);
		if (current_level->tileMap[i] > 1) {
			self->position.y = th * (indices.h) - (self->bounds.y + self->bounds.h);
			self->velocity.y = 0;
			self->isGrounded = 1;
		}
	}
}

void entity_update(Entity *self)
{
	if (!self) return;
	GFC_Rect bounds, indices;
	Level* current_level;
	int i, tw, th;
	
	if (self->update) self->update(self);

	self->position.x += self->velocity.x;
	check_bounds(self, 0);
	self->position.y += self->velocity.y;
	check_bounds(self, 1);

	if (self->velocity.x > GFC_EPSILON)
	{
		self->velocity.x *= 0.5;
	}
	else self->velocity.x = 0;
	
	// Animation stuff, might want to move this into the animation class
	if (!self->animationData) return;
	AnimData* data;
	data = self->animationData;
	self->frame = (float)(data->FrameRow * data->FramesPerRow + data->FrameCol);
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

void entity_load(Entity* ent, char* state)
{
	SJson* json, * config;
	AnimData* animationData;

	if (!ent)
	{
		slog("Invalid entity");
		return;
	}
	if (!state)
	{
		slog("Invalid state");
		return;
	}
	if (!ent->animDataFilePath)
	{
		slog("JSON filepath invalid");
		return;
	}
	json = sj_load(ent->animDataFilePath);
	if (!json)
	{
		slog("JSON data invalid");
		return;
	}

	config = sj_object_get_value(json, "AnimData");
	if (!config)
	{
		slog("Failed to load animation data for %s", ent->animDataFilePath);
		sj_free(json);
		return;
	}

	animationData = animdata_parse(config, state, ent->animationData);
	if (!animationData)
	{
		slog("Failed to parse animation data for %s", ent->animDataFilePath);
		sj_free(json);
		return;
	}

	ent->animationData = animationData;

	sj_free(json);
	return;
}