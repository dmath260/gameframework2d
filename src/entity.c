#include "simple_logger.h"

#include "entity.h"

typedef struct
{
	Entity	*entityList;
	Uint32	entityMax;
}EntityManager;

static EntityManager entityManager = {0};

void entity_manager_close();

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
	if (self->sprite) gf2d_sprite_free(self->sprite);
	memset(self, 0, sizeof(Entity));
}

void entity_draw(Entity *self)
{
	gf2d_sprite_draw(
		self->sprite,
		self->position,
		&self->scale,
		&self->rotationCenter,
		&self->rotation,
		NULL,
		NULL,
		(Uint32)self->frame);
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
		entityManager.entityList[i].think(&entityManager.entityList[i]);
	}
}

void entity_update(Entity *self) {
	if (!self) return;

	if (self->update) self->update(self);

	gfc_vector2d_add(self->position, self->position, self->velocity);
	if (gfc_vector2d_magnitude(self->velocity) > GFC_EPSILON)
	{
		gfc_vector2d_scale(self->velocity, self->velocity, 0.5);
	}
	else gfc_vector2d_clear(self->velocity);
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

void entity_manager_kill_random()
{
	int i;
	// check if entities other than the player can be freed; if not, return
	for (i = 1; i < entityManager.entityMax; i++)
	{
		if (entityManager.entityList[i]._inuse) i = entityManager.entityMax + 1;
	}
	if (i <= entityManager.entityMax) {
		slog("Couldn't free any entities other than the player.");
		return;
	}

	// keep guessing until an entity in use is found
	while (1) {
		i = (gfc_crandom() + 1) * entityManager.entityMax / 2;
		if (i == 0 || !entityManager.entityList[i]._inuse) continue;
		break;
	}
	entity_free(&entityManager.entityList[i]);
}