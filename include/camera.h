#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "gfc_shape.h"

typedef struct
{
	GFC_Rect bounds;
	GFC_Rect view;
}Camera;

/**
 * @brief sets the camera bounds
 * @param bounds the bounds to set
 */
void camera_set_bounds(GFC_Rect bounds);

/**
 * @brief gets the camera bounds
 * @return the camera's current bounds
 */
GFC_Rect camera_get_bounds();

/**
 * @brief gets the current position of the camera
 * @return the camera's current position as a GFC_Vector2D
 */
GFC_Vector2D camera_get_position();

/**
 * @brief gets the current offset of the camera
 * @return the camera's current offset as a GFC_Vector2D
 */
GFC_Vector2D camera_get_offset();

/**
 * @brief sets the position of the camera's top-left corner
 * @param position the position to set the top-left corner of the camera to
 */
void camera_set_position(GFC_Vector2D position);

/**
 * @brief sets the dimensions of the camera
 * @param dimensions the dimensions to set the camera to
 */
void camera_set_dimensions(GFC_Vector2D dimensions);

/**
 * @brief centers the camera on the specified position
 * @param the position to center the camera on
 */
void camera_center_on(GFC_Vector2D position);

#endif