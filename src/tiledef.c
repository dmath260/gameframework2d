#include "simple_logger.h"
#include "simple_json.h"

#include "tiledef.h"

TileDef* tiledef_new()
{
	TileDef* tiledef;
	tiledef = gfc_allocate_array(sizeof(TileDef), 1);
	if (!tiledef) return NULL;
	// default init stuff
	return tiledef;
}

void tiledef_free(TileDef* tiledef)
{
	if (!tiledef) return;

	free(tiledef);
}

TileDef* tiledef_parse(SJson* config)
{
	const char* str;
	TileDef* tiledef;
	if (!config) {
		return NULL;
	}
	tiledef = tiledef_new();

	Uint32 width, height, fpl;
	sj_object_get_uint32(config, "width", &tiledef->width);
	sj_object_get_uint32(config, "height", &tiledef->height);
	sj_object_get_uint32(config, "fpl", &tiledef->fpl);

	str = sj_object_get_string(config, "sheet");
	if (str)
	{
		tiledef->sheet = gf2d_sprite_load_all(
			str,
			tiledef->width,
			tiledef->height,
			tiledef->fpl,
			1);
	}

	return tiledef;
}