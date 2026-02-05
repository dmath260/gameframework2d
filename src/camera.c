#include "simple_logger.h"

#include "camera.h"

static Camera camera = {0};

GFC_Vector2D camera_get_position()
{
	return gfc_vector2d(camera.bounds.x, camera.bounds.y);
}

GFC_Vector2D camera_get_offset()
{
	return gfc_vector2d(-camera.bounds.x, -camera.bounds.y);
}

void camera_set_position(GFC_Vector2D position)
{
	camera.bounds.x = position.x;
	camera.bounds.y = position.y;
}

void camera_set_dimensions(GFC_Vector2D dimensions)
{
	camera.bounds.w = dimensions.x;
	camera.bounds.h = dimensions.y;
}

void camera_center_on(GFC_Vector2D position)
{
	camera.bounds.x = position.x - (camera.bounds.w / 2);
	camera.bounds.y = position.y - (camera.bounds.h / 2);
}