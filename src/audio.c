#include "simple_logger.h"

#include "gfc_audio.h"
#include "gfc_list.h"

#include "audio.h"

typedef struct
{
    Mix_Music* music;
    char* filename;
    int loops;
    double timestamp;
} MusicData;

static GFC_List* music_queue = {0};
static Mix_Music* current_music = {0};

void audio_init(
    Uint32 maxGFC_Sounds,
    Uint32 channels,
    Uint32 channelGroups,
    Uint32 maxMusic,
    Uint8  enableMP3,
    Uint8  enableOgg
)
{
    music_queue = gfc_list_new();
    if (!music_queue) {
        slog("Couldn't initialize music queue");
        return;
    }

    gfc_audio_init(
        maxGFC_Sounds,
        channels,
        channelGroups,
        maxMusic,
        enableMP3,
        enableOgg
    );

    slog("initialized audio system");
}

int play_music(MusicData *music_data)
{
    if (!music_data || !music_data->music) return -1;
    slog("Playing music %s", music_data->filename);
    if (Mix_PlayMusic(music_data->music, music_data->loops)) return -1;
    if (current_music) Mix_FreeMusic(current_music);
    current_music = music_data->music;
    return 0;
}

int play_music_first()
{
    MusicData* music_data;
    music_data = (MusicData*)gfc_list_get_nth(music_queue, 0);
    if (!music_data || !music_data->music) return -1;
    if (play_music(music_data)) return -1;
    gfc_list_delete_nth(music_queue, 0);
    return 0;
}

int enqueue_music(char* filename, int loops)
{
    Mix_Music* music;
    MusicData* music_data;
    void* music_void;
    music = Mix_LoadMUS(filename);
    if (!music) {
        slog("Music couldn't be loaded: %s", filename);
        return -1;
    }
    music_data = gfc_allocate_array(sizeof(MusicData), 1);
    if (!music_data)
    {
        Mix_FreeMusic(music);
        slog("Couldn't allocate music data");
        return -1;
    }
    music_data->music = music;
    music_data->filename = filename;
    music_data->loops = loops;
    music_void = (void *)music_data;
    gfc_list_append(music_queue, music_void);
    if (!Mix_PlayingMusic()) {
        return play_music_first();
    }
    return 0;
}

void music_update()
{
    double remTime;
    remTime = Mix_MusicDuration(current_music) - Mix_GetMusicPosition(current_music);
    if (!Mix_PlayingMusic() || remTime < 1.0 / 120.0 ) {
        if (gfc_list_get_count) play_music_first();
        else if (Mix_PlayingMusic()) slog("Music queue is empty");
    }
}

void music_queue_free()
{
    if (!music_queue) return;
    Uint32 i;
    MusicData* data;
    for (i = 0; i < gfc_list_get_count(music_queue); i++)
    {
        data = (MusicData*)gfc_list_get_nth(music_queue, i);
        if (!data) continue;
        if (data->music) Mix_FreeMusic(data->music);
    }
    gfc_list_clear(music_queue);
    gfc_list_delete(music_queue);
}