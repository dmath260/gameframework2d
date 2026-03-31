#ifndef __AUDIO_H__
#define __AUDIO_H__

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
* @brief adds music to the queue and plays it if nothing else is playing
* @param filename the path to the song to play
* @param loops # of times to loop (0 = no loop, -1 = basically infinite loops)
* @return -1 if failed, 0 if successful
*/
int enqueue_music(char* filename, int loops);

/**
* @brief gets called every frame and updates any necessary music data
*/
void music_update();

/**
* @brief frees the music queue and every element in it
*/
void music_queue_free();

#endif