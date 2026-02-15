#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "simple_json.h"

#include "gf2d_sprite.h"

typedef struct
{
	Sprite* Sprite;			// the currently loaded sprite
	char* Name;				// the name/state of the current animation
	Uint32* AnimFrames;		// total number of frames since start of animation before switching to a given frame
	Uint8 FrameWidth;		// width of each frame in pixels
	Uint8 FrameHeight;		// height of each frame in pixels
	Uint8 FrameRow;			// row of the current frame
	Uint8 FrameCol;			// height of the current frame
	float FrameCount;		// helps indicate which frame in a row to load
	Uint8 FramesPerRow;		// number of frames in a row
}AnimData;

/**
* @brief creates a new animation data struct
* @return NULL if cannot create new animation data struct, pointer to animation data struct otherwise
*/
AnimData* animdata_new();

/**
* @brief frees animation data
* @param data the data to free
*/
void animdata_free(AnimData *data);

/**
* @brief parses animation data from JSON
* @param config the JSON data to parse
* @param state the state to parse animation data for
* @param any existing animation data the entity already has
* @return NULL if JSON is invalid, pointer to animation data struct otherwise
*/
AnimData* animdata_parse(SJson* json, char* state, AnimData* animationData);

#endif