#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "gfc_shape.h"

#include "gf2d_sprite.h"

typedef enum
{
	WET_Label,
	WET_Actor,
	WET_Button,
	WET_CheckBox,
	WET_Slider,
	WET_Container,
	WET_MAX
}WindowElementTypes;

typedef struct
{
	GFC_Rect bound;
}WindowElement;

typedef struct
{
	GFC_Rect bound;
}Window;

#endif