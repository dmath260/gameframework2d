#include "simple_logger.h"

#include "camera.h"

static Camera camera = {0};

void camera_set_bounds(GFC_Rect bounds)
{
	camera.bounds = bounds;
}

GFC_Rect camera_get_bounds()
{
	return camera.bounds;
}

void camera_snap_to_bounds()
{
	if (camera.view.x < camera.bounds.x) camera.view.x = camera.bounds.x;
	if (camera.view.y < camera.bounds.y) camera.view.y = camera.bounds.y;
	if (camera.view.x + camera.view.w > camera.bounds.w) camera.view.x = camera.bounds.w - camera.view.w;
	if (camera.view.y + camera.view.h > camera.bounds.h) camera.view.y = camera.bounds.h - camera.view.h;
}

GFC_Vector2D camera_get_position()
{
	return gfc_vector2d(camera.view.x, camera.view.y);
}

GFC_Vector2D camera_get_offset()
{
	return gfc_vector2d(-camera.view.x, -camera.view.y);
}

void camera_set_position(GFC_Vector2D position)
{
	camera.view.x = position.x;
	camera.view.y = position.y;
	camera_snap_to_bounds();
}

void camera_set_dimensions(GFC_Vector2D dimensions)
{
	camera.view.w = dimensions.x;
	camera.view.h = dimensions.y;
	camera_snap_to_bounds();
}

void camera_center_on(GFC_Vector2D position)
{
	camera.view.x = position.x - (camera.view.w / 2);
	camera.view.y = position.y - (camera.view.h / 2);
	camera_snap_to_bounds();
}