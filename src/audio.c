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
static MusicData* current_music = {0};

void audio_init(
    Uint32 maxGFC_Sounds,
    Uint32 channels,
    Uint32 channelGroups,
    Uint32 maxMusic,
    Uint8  enableMP3,
    Uint8  enableOgg
)
{
    // NOTE: BE CONSISTENT ABOUT AUDIO SAMPLE RATE!
    // Either use all 44.1 kHz, or use all 48 kHz. Nothing else will work.
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
    slog("Playing music %s with %u loops", music_data->filename, music_data->loops);
    if (Mix_PlayMusic(music_data->music, music_data->loops)) return -1;
    if (current_music) {
        Mix_FreeMusic(current_music->music);
        free(current_music);
    }
    current_music = music_data;
    return 0;
}

void pause_music()
{
    Mix_PauseMusic();
}

void resume_music()
{
    Mix_ResumeMusic();
}

void toggle_music()
{
    if (Mix_PausedMusic()) Mix_ResumeMusic();
    else Mix_PauseMusic();
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

char* get_current_music_filename()
{
    if (!current_music || !current_music->filename) return NULL;
    return current_music->filename;
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

void load_level_music(Level *level)
{
    if (!level) return;
    load_music_pair(level->music_intro, level->music_loop);
}

void load_music_pair(const char* intro, const char* loop)
{
    const char* str;
    if (intro || loop)
    {
        str = get_current_music_filename();
        // only replace the music if there is no song playing or the song is different
        if (
            !str ||
            !(intro && !strcmp(intro, str)) &&
            !(loop && !strcmp(loop, str))
        )
        {
            music_queue_clear();
            if (intro) enqueue_music(_strdup(intro), 0);
            if (loop) enqueue_music(_strdup(loop), -1);
            if (str) play_music_first();
        }
    }
}

void music_update()
{
    double remTime;
    remTime = Mix_MusicDuration(NULL) - Mix_GetMusicPosition(NULL);
    if (!Mix_PlayingMusic() || remTime < 1.0 / 120.0 ) {
        if (gfc_list_get_count) play_music_first();
        else if (Mix_PlayingMusic()) slog("Music queue is empty");
    }
}

void music_queue_clear()
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
}

void music_queue_free()
{
    music_queue_clear();
    gfc_list_delete(music_queue);
}