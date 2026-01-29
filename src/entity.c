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
	if (self->sprite) gf2d_sprite_free(self->sprite);
	memset(self, 0, sizeof(Entity));
}

void entity_draw(Entity *self)
{
	gf2d_sprite_draw(
		self->sprite,
		self->position,
		&self->scale,
		NULL,
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