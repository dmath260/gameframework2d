#include "simple_logger.h"

#include "monster.h"
#include "player.h"
#include "level.h"

#include "monster_grunt.h"
#include "monster_seeker.h"
#include "monster_gunner.h"
#include "monster_flier.h"
#include "monster_immortalsnail.h"



char* monster_state_to_str(Entity* self)
{
	if (!self || !self->data) return "Idle";
	MonsterStates state;
	MonsterData* data;
	data = (MonsterData*)self->data;
	state = data->state;
	switch (state)
	{
		case MS_Idle: return "Idle";
		case MS_Walk: return "Walk";
		case MS_Attack: return "Shoot";
		case MS_Pain: return "Pain";
		case MS_Die: return "Faint";
		default: return "Idle";
	}
}

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
	Level *currentLevel;
	if ((!self) || (!self->data)) return;
	data = (MonsterData*)self->data;
	entity_collision_test_world(self);

	currentLevel = get_current_level();
	if (!currentLevel) return;
	if (self->position.y + self->bounds.y > currentLevel->height * currentLevel->tileDef->height)
	{
		self->health = 0;
	}
}

Uint8 monster_touch(Entity* self, Entity* other)
{
	MonsterData* data;
	if (!self | !other) return 0;
	data = (MonsterData*) self->data;
	if (other == data->player)
	{
		//self->health -= self->maxHealth;
		return 1;
	}
	return 1;
}

Entity* monster_new(GFC_Vector2D position, MonsterTypes type)
{
	Entity* self;
	MonsterData* data;
	char* state;
	self = entity_new();
	if (!self) return NULL;
	data = gfc_allocate_array(sizeof(MonsterData), 1);
	if (data)
	{
		data->player = player_entity_get();
	}
	self->data = data;
	self->position = position;
	self->thinkPos = position;
	self->team = 1; // team 1 for monsters
	state = "Idle";
	switch (type) {
		case MT_Grunt:
			monster_grunt_populate(self);
			state = "Walk";
			break;
		case MT_Seeker:
			monster_seeker_populate(self);
			break;
		case MT_Gunner:
			monster_gunner_populate(self);
			break;
		case MT_Flier:
			monster_flier_populate(self);
			state = "Shoot";
			break;
		case MT_ImmortalSnail:
			monster_immortalsnail_populate(self);
			break;
		default:
			monster_grunt_populate(self);
			state = "Walk";
			break;
	}
	entity_load(self, state);
	self->health = self->maxHealth;
	self->maxIFrames = 45;
	self->animationData->FrameRow = 2;
	return self;
}