#include "simple_logger.h"

#include "bullet.h"

void bullet_free(Entity* self)
{
	if ((!self)) return;
}

void bullet_think(Entity* self)
{
	if (!self) return;
	self->thinkPos.x += self->topSpeed;
}

void bullet_update(Entity* self)
{
	if (!self) return;
	entity_collision_test_world(self);
}

Uint8 bullet_touch(Entity* self, Entity* other)
{
	int otherHealth;
	if (!self || !other || other->team == 2) return 0;
	if (other->team != self->team)
	{
		self->health = 0;
		otherHealth = other->health;
		if (other->touch && (self->maxHealth > 1 || other->maxHealth > 1))
		{
			other->touch(other, self); // condition prevents crash for two bullets
		}
		if (other->health == otherHealth)
		{
			entity_hurt(other, self->attack); // don't do damage twice
		}
		return 1;
	}
	return 1;
}

Entity* bullet_new(GFC_Vector2D position, GFC_Color color, Uint8 team, Uint8 dir, float power)
{
	Entity* self;
	self = entity_new();
	if (!self) return NULL;
	self->animDataFilePath = "images/bullet/bulletAnimData.json";
	entity_load(self, "Bullet");
	self->animationData->FrameRow = 0;
	self->bounds = gfc_rect(0, 0, 12, 8);
	self->color = color;
	self->team = team;
	self->topSpeed = 5;
	self->topSpeed -= self->topSpeed * 2 * dir;
	self->speedMult = 1;
	self->gravity = 0;
	self->maxHealth = 1;
	self->health = self->maxHealth;
	self->attack = power;
	self->position = position;
	self->thinkPos = position;
	self->think = bullet_think;
	self->update = bullet_update;
	self->touch = bullet_touch;
	self->scale = gfc_vector2d(4, 4);
	self->free = bullet_free;
	return self;
}