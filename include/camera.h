#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "gfc_shape.h"

typedef struct
{
	GFC_Rect bounds;
}Camera;

GFC_Vector2D camera_get_position();

GFC_Vector2D camera_get_offset();

void camera_set_position(GFC_Vector2D position);

void camera_set_dimensions(GFC_Vector2D dimensions);

void camera_center_on(GFC_Vector2D position);

#endif