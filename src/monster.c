#include "simple_logger.h"

#include "monster.h"
#include "player.h"

#include "monster_grunt.h"
#include "monster_seeker.h"
#include "monster_gunner.h"
#include "monster_flier.h"
#include "monster_immortalsnail.h"

void monster_free(Entity* self)
{
	MonsterData* data;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;
	//clean up anything I own that I asked for
	free(data);
}

void monster_think(Entity* self)
{
	MonsterData* data;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;
}

void monster_update(Entity* self)
{
	MonsterData* data;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;
	entity_collision_test_world(self);
}

Uint8 monster_touch(Entity* self, Entity* other)
{
	MonsterData* data;
	if (!self | !other) return 0;
	data = (MonsterData*) self->data;
	if (other == data->player)
	{
		entity_free(self);
		return 1;
	}
	return 1;
}

Entity* monster_new(GFC_Vector2D position, MonsterTypes type)
{
	Entity* self;
	MonsterData* data;
	self = entity_new();
	if (!self) return NULL;
	data = gfc_allocate_array(sizeof(MonsterData), 1);
	if (data)
	{
		data->player = player_entity_get();
	}
	self->data = data;
	self->position = position;
	switch (type) {
		case MT_Grunt:
			monster_grunt_populate(self);
			break;
		case MT_Seeker:
			monster_seeker_populate(self);
			break;
		case MT_Gunner:
			monster_gunner_populate(self);
			break;
		case MT_Flier:
			monster_flier_populate(self);
			break;
		case MT_ImmortalSnail:
			monster_immortalsnail_populate(self);
			break;
		default:
			monster_grunt_populate(self);
			break;
	}
	entity_load(self, "Idle");
	return self;
}