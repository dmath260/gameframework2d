#include "simple_logger.h"

#include "animation.h"

AnimData* animdata_new()
{
	AnimData* data;
	data = gfc_allocate_array(sizeof(AnimData), 1);
	if (!data) return NULL;
	return data;
}

AnimData* animdata_parse(SJson* config, char* state, AnimData* animationData)
{
	int i, count;
	SJson *temp, *array, *data, *duration;
	Uint32 value;
	const char *str;
	// don't need to check for other parameters being valid, only called in functions that already check them
	if (!animationData) {
		animationData = animdata_new();
		if (!animationData) {
			slog("Could not create animation data");
			return NULL;
		}
	}

	temp = sj_object_get_value(config, "Anims");
	if (!temp) {
		return NULL;
	}
	array = sj_object_get_value(temp, "Anim");
	if (!array) {
		return NULL;
	}
	count = sj_array_count(array);
	for (i = 0; i < count; i++) {
		data = sj_array_nth(array, i);
		str = sj_object_get_string(data, "Name");
		if (!strcmp(str, state)) break;
	}
	if (i == count) {
		return NULL;
	}

	data = sj_array_nth(array, i);
	animationData->Name = state;
	sj_object_get_uint8(data, "FrameWidth", &animationData->FrameWidth);
	sj_object_get_uint8(data, "FrameHeight", &animationData->FrameHeight);
	animationData->FrameCol = 0;
	animationData->FrameCount = 0;

	temp = sj_object_get_value(data, "Durations");
	if (!temp) {
		return NULL;
	}
	duration = sj_object_get_value(temp, "Duration");
	if (!duration) {
		return NULL;
	}
	count = sj_array_count(duration);
	animationData->FramesPerRow = count;
	animationData->AnimFrames = gfc_allocate_array(sizeof(Uint32), count + 1);
	if (!animationData->AnimFrames) {
		return NULL;
	}
	value = 0;
	for (i = 0; i < count; i++)
	{
		temp = sj_array_nth(duration, i);
		sj_get_uint32_value(temp, &value);
		animationData->AnimFrames[i + 1] = animationData->AnimFrames[i] + value;
	}


	str = sj_object_get_string(data, "Filepath");
	animationData->Sprite = gf2d_sprite_load_all(
		str,
		animationData->FrameWidth,
		animationData->FrameHeight,
		count,
		0
	);
	if (!animationData->Sprite) {
		return NULL;
	}

	return animationData;
}

void animdata_free(AnimData *data)
{
	if (data->Sprite) {
		gf2d_sprite_free(data->Sprite);
		data->Sprite = NULL;
	}
	if (data->AnimFrames) {
		free(data->AnimFrames);
		data->AnimFrames = NULL;
	}
	free(data);
}