#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "level.h"

/**
* @brief initializes the audio system
* @param maxGFC_Sounds the maximum number of sounds that can be loaded into memory at once
* @param channels the nrumber of allocated audio channels (excluding music channel)
* @param channelGroups the number of channels to be reserved for groups to be set up
* @param maxMusic the number of simultaneous music files that will be supported
* @param enableMP3 if true, initializes audio system with mp3 support, if available
* @param enableOgg if true, initializes audio system with ogg vorbis support, if available
*/
void audio_init(
    Uint32 maxGFC_Sounds,
    Uint32 channels,
    Uint32 channelGroups,
    Uint32 maxMusic,
    Uint8  enableMP3,
    Uint8  enableOgg
);

/**
* @brief pauses the currently playing music
*/
void pause_music();

/**
* @brief resumes any paused music
*/
void resume_music();

/**
* @brief toggles music between playing and paused
*/
void toggle_music();

/**
* @brief plays the first song in the music queue
* @return -1 on failure, 0 on success
*/
int play_music_first();

/**
* @brief gets the filename of the song that's currently playing
* @return a pointer to the filename of the song that's currently playing or NULL if invalid
*/
char* get_current_music_filename();

/**
* @brief adds music to the queue and plays it if nothing else is playing
* @param filename the path to the song to play
* @param loops # of times to loop (0 = no loop, -1 = basically infinite loops)
* @return -1 if failed, 0 if successful
*/
int enqueue_music(char* filename, int loops);

/**
* @brief loads music from a level (intro and loop)
* @param level the level to load music from
*/
void load_level_music(Level* level);

/**
* @brief loads music from intro and loop filenames
* @param intro the filename for the music intro
* @param loop the filename for the music loop
*/
void load_music_pair(const char *intro, const char *loop);

/**
* @brief gets called every frame and updates any necessary music data
*/
void music_update();

/**
* @brief frees every element in the music queue, but not the music queue itself
*/
void music_queue_clear();

/**
* @brief frees the music queue and every element in it
*/
void music_queue_free();

#endif