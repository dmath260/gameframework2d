#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_hashmap.h"

#include "gf2d_draw.h"
#include "gf2d_windows.h"

#include "entity.h"
#include "camera.h"
#include "level.h"
#include "player.h"
#include "item.h"

typedef struct
{
	Entity		*entityList;
	Uint32		entityMax;
	Uint32		entityPool;
	GFC_HashMap	*name_to_id;
}EntityManager;

static EntityManager entityManager = {0};
static GFC_List *entity_id_list = {0};

void entity_manager_close(void);

EntityEntry* new_entity_entry(const char* name, Uint8 id, const char* icon)
{
	EntityEntry *entry;
	if (!name || !icon) return NULL;
	entry = (EntityEntry *)gfc_allocate_array(sizeof(EntityEntry), 1);
	if (!entry) return NULL;
	entry->name = name;
	entry->id = id;
	entry->icon = gf2d_sprite_load_image(icon);
	return entry;
}

EntityEntry* get_entity_data_at_id(Uint8 id)
{
	EntityEntry* entry;
	entry = (EntityEntry*)(gfc_list_get_nth(entity_id_list, id));
	if (!entry) return NULL;
	return entry;
}

void entity_manager_init(Uint32 max)
{
	if (!max)
	{
		return;
	}
	entityManager.entityList = gfc_allocate_array(sizeof(Entity), max);
	if (!entityManager.entityList)
	{
		return;
	}
	entityManager.entityMax = max;
	atexit(entity_manager_close);

	entity_id_list = gfc_list_new_size(256);
	if (!entity_id_list)
	{
		return;
	}
	entity_id_list->count = 256; //manual override so list methods work
	entityManager.name_to_id = gfc_hashmap_new();

	SJson *json, *config, *array, *entry, *id_json;
	const char *name, *icon;
	Uint8 id;
	int m, i;
	EntityEntry* new_entry;

	json = sj_load("config/entity_ids.cfg");
	if (!json)
	{
		return;
	}
	config = sj_object_get_value(json, "entities");
	if (!config)
	{
		sj_free(json);
		return;
	}

	m = sj_array_get_count(config);
	if (!m) return;
	for (i = 0; i < m; i++)
	{
		entry = sj_array_get_nth(config, i);
		name = _strdup(sj_object_get_string(entry, "name"));
		id_json = sj_object_get_value(entry, "id");
		icon = _strdup(sj_object_get_string(entry, "icon"));
		if (!name || !icon || !id_json) continue;
		sj_get_uint8_value(id_json, &id);
		new_entry = new_entity_entry(name, id, icon);
		gfc_list_set_nth(entity_id_list, id, (void*)new_entry);
	}

	sj_free(json);
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

	EntityEntry* data;
	for (i = 0; i < gfc_list_get_count(entity_id_list); i++)
	{
		data = (EntityEntry*)gfc_list_get_nth(entity_id_list, i);
		if (!data) continue;
		if (data->name) free(data->name);
		if (data->icon) gf2d_sprite_free(data->icon);
	}
	gfc_list_clear(entity_id_list);
	free(entity_id_list);
	gfc_hashmap_free(entityManager.name_to_id);

	memset(&entityManager, 0, sizeof(EntityManager));
}

void entity_manager_free_all_but_player()
{
	Uint32 i;
	if (!entityManager.entityList) return;
	for (i = 0; i < entityManager.entityMax; i++)
	{
		if (&(entityManager.entityList[i]) == player_entity_get()) continue;
		if ((entityManager.entityList[i]).isClimbing == 42) continue;
		entity_free(&entityManager.entityList[i]);
	}
}

Entity* entity_new()
{
	Uint32 i;
	if (!entityManager.entityList) {
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
	if (self->item && self->team != 2 && self != player_entity_get()) {
		self->position.x -= self->bounds.w / 2;
		self->position.y -= self->bounds.h / 2;
		item_new(self->position, self->item);
	}
	else if (!self->item && self->team == 1 && self->maxHealth <= 20 && self->maxHealth > 1 && gfc_random() < 0.25)
	{
		self->position.x -= self->bounds.w / 2;
		self->position.y -= self->bounds.h / 2;
		item_new(self->position, IT_HealthRestore);
	}
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
	gf2d_draw_rect(bounds, gfc_color8(128, 0, 255, 255));
}

void entity_manager_draw_all()
{
	Uint32 i;
	if (!entityManager.entityList) {
		return;
	}
	for (i = 0; i < entityManager.entityMax; i++)
	{
		if (!entityManager.entityList[i]._inuse) continue;
		if (entityManager.entityList[i].isClimbing == 42) continue;
		entity_draw(&entityManager.entityList[i]);
	}
}

float binary_search_position(Entity *self, Uint8 axis, int start, int end)
{
	float start_pos, target_pos, mid, offset, lower, upper;
	int i, j, index, tw, th, tr, coll;
	Level* current_level;

	current_level = get_current_level();
	if (!self || !current_level) return NAN;
	if (axis)
	{
		offset = (self->velocity.y > 0) ? self->bounds.y + self->bounds.h : self->bounds.y;
		start_pos = self->position.y + offset;
		target_pos = self->thinkPos.y + offset;
	}
	else
	{
		offset = (self->velocity.x > 0) ? self->bounds.x + self->bounds.w : self->bounds.x;
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
			if (axis) index = level_get_tile_index(current_level, j, (Uint32)(mid / th));
			else index = level_get_tile_index(current_level, mid / tw, j);
			if (index >= 0 && current_level->tileMap[index] > 0) {
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
	if (target_pos > start_pos)
	{
		tr = (int)(mid / th);
		while (!coll && tr < (int)current_level->height)
		{
			for (j = start; j <= end; j++)
			{
				index = level_get_tile_index(current_level, j, tr);
				if (index >= 0 && current_level->tileMap[index] > 0)
				{
					if (!tile_is_solid(current_level->tileMap[i], 0)) continue;
					coll = 1;
					break;
				}
			}
			if (!coll) tr++;
		}
	}
	else
	{
		tr = (int)(mid - 0.001f / th);
		while (!coll && tr >= 0)
		{
			for (j = start; j <= end; j++)
			{
				index = level_get_tile_index(current_level, j, tr);
				if (index >= 0 && current_level->tileMap[index] > 0)
				{
					if (!tile_is_solid(current_level->tileMap[i], 0)) continue;
					coll = 1;
					break;
				}
			}
			if (!coll) tr--;
		}
	}
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
				if (i >= 0 && current_level->tileMap[i] > 0)
				{
					if (!tile_is_solid(current_level->tileMap[i], 0)) continue;
					return 1;
				}
			}
		}
		else if (self->velocity.x > 0)
		{
			for (y = indices.y; y <= indices.h; y++)
			{
				i = level_get_tile_index(current_level, indices.w, y);
				if (i >= 0 && current_level->tileMap[i] > 0)
				{
					if (!tile_is_solid(current_level->tileMap[i], 0)) continue;
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
				int ycheck = (int)((bounds.y + bounds.h) / th);
				i = level_get_tile_index(current_level, x, indices.h);
				if (i >= 0 && current_level->tileMap[i] > 0)
				{
					if (!tile_is_solid(current_level->tileMap[i], 0)) continue;
					return 1;
				}
			}
			else if (self->velocity.y < 0)
			{
				i = level_get_tile_index(current_level, x, indices.y);
				if (i >= 0 && current_level->tileMap[i] > 0)
				{
					if (!tile_is_solid(current_level->tileMap[i], 0)) continue;
					return 1;
				}
			}
			else
			{
				int ycheck = (int)((bounds.y + bounds.h) / th);
				i = level_get_tile_index(current_level, x, ycheck);
				if (i >= 0 && current_level->tileMap[i] > 0)
				{
					if (!tile_is_solid(current_level->tileMap[i], 0)) continue;
					return 1;
				}
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
	float snapped;
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

		if (self->velocity.x < 0)
		{
			for (y = indices.y; y <= indices.h; y++)
			{
				i = level_get_tile_index(current_level, indices.x, y);
				if (i >= 0 && current_level->tileMap[i] > 0)
				{
					if (!tile_is_solid(current_level->tileMap[i], 1)) continue;
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
				if (i >= 0 && current_level->tileMap[i] > 0)
				{
					if (!tile_is_solid(current_level->tileMap[i], 1)) continue;
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

		for (x = indices.x; x <= indices.w; x++)
		{
			if (self->velocity.y > 0)
			{
				i = level_get_tile_index(current_level, x, indices.h);
				if (i >= 0 && current_level->tileMap[i] > 0)
				{
					if (!tile_is_solid(current_level->tileMap[i], 1)) continue;
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
						else if (!self->impulse) self->velocity.y = -5;
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
				if (i >= 0 && current_level->tileMap[i] > 0)
				{
					if (!tile_is_solid(current_level->tileMap[i], 1)) continue;
					if (current_level->tileMap[i] == 73)
					{
						self->isClimbing = 1;
						continue;
					}
					self->isClimbing = 0;
					snapped = binary_search_position(self, axis, indices.x, indices.w);
					self->thinkPos.y = (snapped > self->position.y) ? self->position.y : snapped;
					self->velocity.y = 0;
					if (current_level->tileMap[i] == 65) entity_hurt(self, 1);
					if (current_level->tileMap[i] == 71 || current_level->tileMap[i] == 72)
					{
						if (!self->impulse) break;
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
						gf2d_windows_play_sound("switch");
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
		if (entityManager.entityList[i].isClimbing == 42) continue;
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
	Uint8 wasGrounded;
	if (!self) return;

	wasGrounded = self->isGrounded;
	self->isGrounded = 0;
	self->isClimbing = 0;

	//downward velocity (if entity is affected by gravity)
	if (self->gravity)
	{
		if (!self->isClimbing && !wasGrounded) {
			self->velocity.y += 0.25;
		}
		self->thinkPos.y += self->velocity.y;
		clip_to_bounds(self, 1);
		if (!self->isGrounded && wasGrounded && self->velocity.y == 0)
		{
			if (check_bounds(self, 1)) {
				self->isGrounded = 1;
				// axis: 0 for x, anything else for y
				if (!self) return;
				GFC_Rect bounds;
				Level* current_level;
				int i, tw, th, x;

				bounds = self->bounds;
				gfc_vector2d_add(bounds, bounds, self->thinkPos);
				current_level = get_current_level();
				tw = current_level->tileDef->width;
				th = current_level->tileDef->height;

				for (x = (int)(bounds.x / tw); x <= (int)((bounds.x + bounds.w - 1) / tw); x++)
				{
					i = level_get_tile_index(current_level, x, (int)((bounds.y + bounds.h) / th));
					if (i < 0 && current_level->tileMap[i] <= 0) continue;
					if (current_level->tileMap[i] == 65)
					{
						entity_hurt(self, 1);
						break;
					}
					if (current_level->tileMap[i] == 66)
					{
						if (!self->gravity) break;
						else if (!self->impulse) self->velocity.y = -5;
						else if (self == player_entity_get() && gfc_input_key_down("w"))
							self->velocity.y = self->impulse * sqrt(2);
						else self->velocity.y = self->impulse / sqrt(2);
						self->isGrounded = 0;
						break;
					}
				}
			}
			else self->thinkPos.y = self->position.y;
		}
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

	if (self->itemFrames) {
		if (self->itemFrames == 1)
		{
			self->color = GFC_COLOR_WHITE;
			self->item = IT_NONE;
			gf2d_windows_play_sound("expire");
		}
		self->itemFrames--;
	}

	if (self->iFrames || (self->item == IT_Invincible && self == player_entity_get())) {
		if (self->item == IT_Invincible && self == player_entity_get())
		{
			self->iFrames = self->itemFrames;
			if (self->color.r >= 360) self->color.r -= 360;
			self->color = gfc_color_hsl(self->color.r + M_PI, 1, 0.5, 1);
		}
		else {
			self->iFrames--;
		}
		if (self->iFrames == 0) {
			self->color.r *= 2;
			self->color.g *= 2;
			self->color.b *= 2;
		}
	}
	else if (self == player_entity_get())
	{
		switch (self->item)
		{
			case IT_Power:
				self->color = gfc_color_hsl(0, 1, 0.5, 1); // used HSL so invincible can be rainbow
				break;
			case IT_Speed:
				self->color = gfc_color(0, 0.5, 1, 1);
				break;
			case IT_DoubleJump:
				self->color = GFC_COLOR_DARKGREEN;
				break;
			case IT_Hover:
				self->color = GFC_COLOR_YELLOW;
				break;
			case IT_Invincible: break;
			default: self->color = GFC_COLOR_WHITE;
		}
	}
	
	// Animation stuff, might want to move this into the animation class
	if (!self->animationData) return;
	AnimData* data;
	data = self->animationData;
	self->frame = (float)(data->FrameRow * data->FramesPerRow + data->FrameCol);

	if (self->update) self->update(self);
	if (!self) return;

	current_level = get_current_level();
	if (!current_level)
	{
		return;
	}
	if (self->position.x < 0 || self->position.x > current_level->width * current_level->tileDef->width)
	{
		self->health = 0;
	}

	// Last thing: check if entity is still alive
	if (self->health <= 0) {
		if (self == player_entity_get()) player_kill();
		else entity_free(self);
	}
}

void entity_manager_update_all()
{
	Uint32 i;
	if (!entityManager.entityList) {
		return;
	}
	for (i = 0; i < entityManager.entityMax; i++)
	{
		if (!entityManager.entityList[i]._inuse) continue;
		if (!entityManager.entityList[i].update) continue;
		entity_update(&entityManager.entityList[i]);
		if (!get_current_level())
		{
			return;
		}
	}
}

void entity_hurt(Entity* self, Uint8 damage)
{
	if (self->iFrames || self->team == 2) return;
	self->health -= damage;
	if (self->health <= 0) return;
	self->iFrames = self->maxIFrames;
	// replace this w/ pain animation later
	self->color.r /= 2;
	self->color.g /= 2;
	self->color.b /= 2;
	if (self == player_entity_get()) gf2d_windows_play_sound("hurt1");
	else gf2d_windows_play_sound("hurt2");
}

void entity_load(Entity* self, char* state)
{
	SJson* json, * config;
	AnimData* animationData;

	if (!self)
	{
		return;
	}
	if (!state)
	{
		return;
	}
	if (!self->animDataFilePath)
	{
		return;
	}
	json = sj_load(self->animDataFilePath);
	if (!json)
	{
		return;
	}

	config = sj_object_get_value(json, "AnimData");
	if (!config)
	{
		sj_free(json);
		return;
	}

	animationData = animdata_parse(config, state, self->animationData);
	if (!animationData)
	{
		sj_free(json);
		return;
	}

	self->animationData = animationData;

	sj_free(json);
	return;
}

int load_reference_ent(const char* filepath)
{
	SJson* json, * config, * arr, * elem;
	Entity* ent;
	float x, y, w, h;
	json = sj_load(filepath);
	if (!json)
	{
		return 0;
	}

	config = sj_object_get_value(json, "entityInfo");
	if (!config)
	{
		sj_free(json);
		return 0;
	}

	ent = entity_new();
	if (!ent)
	{
		sj_free(json);
		return 0;
	}

	ent->isClimbing = 42; // flag meaning that this entity is a reference entity

	sj_object_get_uint8(config, "team", &(ent->team));
	sj_object_get_uint8(config, "gravity", &(ent->gravity));
	sj_object_get_float(config, "impulse", &(ent->impulse));
	sj_object_get_float(config, "topSpeed", &(ent->topSpeed));
	arr = sj_object_get_value(config, "scale");
	if (arr)
	{
		elem = sj_array_get_nth(arr, 0);
		sj_get_float_value(elem, &x);
		elem = sj_array_get_nth(arr, 1);
		sj_get_float_value(elem, &y);
		ent->scale = gfc_vector2d(x, y);
		x = 0;
		y = 0;
	}
	arr = sj_object_get_value(config, "rotationCenter");
	if (arr)
	{
		elem = sj_array_get_nth(arr, 0);
		sj_get_float_value(elem, &x);
		elem = sj_array_get_nth(arr, 1);
		sj_get_float_value(elem, &y);
		ent->rotationCenter = gfc_vector2d(x, y);
		x = 0;
		y = 0;
	}
	sj_object_get_int32(config, "maxHealth", &(ent->maxHealth));
	sj_object_get_uint8(config, "maxIFrames", &(ent->maxIFrames));
	sj_object_get_uint8(config, "attack", &(ent->attack));
	sj_object_get_int32(config, "maxCooldown", &(ent->maxCooldown));
	arr = sj_object_get_value(config, "bounds");
	if (arr)
	{
		elem = sj_array_get_nth(arr, 0);
		sj_get_float_value(elem, &x);
		elem = sj_array_get_nth(arr, 1);
		sj_get_float_value(elem, &y);
		elem = sj_array_get_nth(arr, 2);
		sj_get_float_value(elem, &w);
		elem = sj_array_get_nth(arr, 3);
		sj_get_float_value(elem, &h);
		ent->bounds = gfc_rect(x, y, w, h);
	}
	ent->animDataFilePath = _strdup(sj_object_get_string(config, "animDataFilePath"));
	entity_load(ent, _strdup(sj_object_get_string(config, "defaultState")));
	sj_object_get_uint8(config, "frameRow", &(ent->animationData->FrameRow));

	gfc_hashmap_insert(entityManager.name_to_id, filepath, (void*)(ent));

	sj_free(json);
	return 1;
}

int populate_entity(Entity* self, const char* filepath)
{
	Entity* ent;
	ent = (Entity*)(gfc_hashmap_get(entityManager.name_to_id, filepath));
	if (!ent)
	{
		if (!load_reference_ent(filepath)) return 0;
		ent = (Entity*)gfc_hashmap_get(entityManager.name_to_id, filepath);
		if (!ent)
		{
			return 0;
		}
	}
	self->team = ent->team;
	self->gravity = ent->gravity;
	self->impulse = ent->impulse;
	self->topSpeed = ent->topSpeed;
	self->scale = ent->scale;
	self->rotationCenter = ent->rotationCenter;
	self->maxHealth = ent->maxHealth;
	self->maxIFrames = ent->maxIFrames;
	self->attack = ent->attack;
	self->maxCooldown = ent->maxCooldown;
	self->bounds = ent->bounds;
	self->animDataFilePath = ent->animDataFilePath;
	entity_load(self, ent->animationData->Name);
	self->animationData->FrameRow = ent->animationData->FrameRow;
	return 1;
}