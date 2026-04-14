#include <SDL.h>
#include "simple_logger.h"

#include "gfc_input.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "gf2d_mouse.h"
#include "gf2d_windows_common.h"
#include "gf2d_font.h"

#include "camera.h"
#include "audio.h"
#include "entity.h"
#include "player.h"
#include "monster.h"
#include "level.h"

static int _done = 0;
static int _paused = 0;
static Window* _pause = NULL;
static Window* _ex = NULL;

void toggle_pause(void* data);

void open_skills(void* data)
{
    return;
}

void onCancel(void* data)
{
    _ex = NULL;
    if (!_pause) toggle_pause(NULL);
}

void onExit(void* data)
{
    _done = 1;
    _ex = NULL;
}

void exit_window(void* data)
{
    _ex = window_yes_no("Exit?", onExit, onCancel, NULL);
}

void toggle_pause(void* data)
{
    if (!_paused)
    {
        _paused = 1;
        if (!_ex) _pause = window_menu(
            "Pause",
            toggle_pause, "Continue",
            open_skills, "View Skills",
            exit_window, "Return to Menu",
            NULL
        );
    }
    else
    {
        _paused = 0;
        _pause = NULL;
    }
    toggle_music();
}

int main(int argc, char * argv[])
{
    /*variable declarations*/
    const Uint8 * keys;
    Level *level;
    
    /*program initialization*/
    init_logger("gf2d.log",0);
    slog("---==== BEGIN ====---");
    gf2d_graphics_initialize(
        "gf2d",
        1200,
        720,
        1200,
        720,
        gfc_vector4d(0,0,0,255),
        0);
    gf2d_graphics_set_frame_delay(16);
    audio_init(32, 16, 4, 4, true, false);
    gf2d_sprite_init(1024);
    gf2d_actor_init(128);
    gf2d_font_init("config/font.cfg");
    gfc_input_init("config/input.cfg");
    gf2d_windows_init(128, "config/windows.cfg");
    entity_manager_init(1024);
    camera_set_dimensions(gfc_vector2d(1200, 720));
    gf2d_mouse_load("actors/mouse.actor");
    SDL_ShowCursor(SDL_DISABLE);

    slog("press [escape] to quit");

    level = level_load("level/level1.json");
    //level = level_load_bin("level/level1.bin");
    if (level)
    {
        //level_save_bin(level, "level/level1.bin");
    }

    Entity* player;
    player = player_entity_get();
    if (!player) player = player_entity_new(gfc_vector2d(
        (float)level->width * level->tileDef->width / 2,
        (float)level->height * level->tileDef->height / 2)
    );

    /*main game loop*/
    while(!_done)
    {
        gfc_input_update(); // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        gf2d_mouse_update();
        gf2d_windows_update_all();

        // pausing
        if (gfc_input_key_pressed("q") && !_paused) toggle_pause(NULL);

        if (!_paused)
        {
            entity_manager_think_all();
            entity_manager_update_all();
            music_update(); // move outside of if statement to continue music when paused
        }
        
        gf2d_graphics_clear_screen();// clears drawing buffers
        // all drawing should happen between clear_screen and next_frame
            //backgrounds drawn first
            if (!_paused)
            {
                level = get_current_level();
                if (level && level->background) level_draw(level);

                entity_manager_draw_all();
                if (player && player->_inuse) entity_draw(player);

                hud_update(player);
            }
            else if (level->background)
                gf2d_sprite_draw_image(level->background, gfc_vector2d(0, 0));
            
            //UI elements last
            gf2d_windows_draw_all();

            gf2d_mouse_draw();

        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame

        if (player && player->_inuse &&
            player->position.y + player->bounds.y > level->height * level->tileDef->height)
        {
            player_kill("Player fell from a high place");
        }
        if (!player || !player->_inuse)
        {
            _done = 1;
        }
        
        if (keys[SDL_SCANCODE_ESCAPE] && _ex == NULL && !_paused)
        {
            exit_window(NULL);
            toggle_pause(NULL);
        }

        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    slog("---==== END ====---");
    music_queue_free();
    return 0;
}
/*eol@eof*/
