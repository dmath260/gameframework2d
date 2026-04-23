#include <SDL.h>
#include "simple_logger.h"

#include "gfc_input.h"

#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "gf2d_mouse.h"
#include "gf2d_windows_common.h"
#include "gf2d_font.h"

#include "camera.h"
#include "audio.h"
#include "editor.h"
#include "entity.h"
#include "player.h"
#include "monster.h"
#include "level.h"

static int _done = 0;
static int _paused = 0;
static Window* _menu = NULL;
static Window* _ex = NULL;
Level* _level;
Entity* _player;

void toggle_pause(void* data);

void open_skills(void* data)
{
    return;
}

void load_level(void* data)
{
    gf2d_window_free(_menu);
    _ex = NULL;
    level_load("level/level1.json", 1);
    //level_save_bin(_level, "level/level1.bin");

    _player = player_entity_get();
    if (!_player) _player = player_entity_new(gfc_vector2d(
        (float)_level->width * _level->tileDef->width / 2,
        (float)_level->height * _level->tileDef->height / 2)
    );
    _level = get_current_level();
    _done = 0;
}

void on_exit(void* data)
{
    _done = 1;
    _ex = NULL;
}

void on_cancel(void* data)
{
    _ex = NULL;
    if (!_menu || (!_level && _paused)) toggle_pause(NULL);
}

void no_save(void* data)
{
    _ex = window_yes_no("No save data found.", "Create a new game?", load_level, on_cancel, NULL);
}

void exit_window(void* data)
{
    _ex = window_yes_no("Exit the game?", NULL, on_exit, on_cancel, NULL);
}

void load_main_menu()
{
    _menu = window_menu(
        "Menu",
        load_level, "New Game",
        no_save, "Continue",
        exit_window, "Quit Game",
        NULL
    );
    load_music_pair("audio/music/title_intro.mp3", "audio/music/title_loop.mp3");
}

void return_to_menu(void* data)
{
    entity_free(_player);
    _level = NULL;
    if (_menu) gf2d_window_free(_menu);
    on_cancel(data);
    load_main_menu();
}

void return_prompt(void* data)
{
    _ex = window_yes_no("Return to the menu?", "(You will lose any unsaved data.)", return_to_menu, on_cancel, NULL);
}

void toggle_pause(void* data)
{
    if (!_paused)
    {
        slog("Pausing game");
        _paused = 1;
        if (!_ex) _menu = window_menu(
            "Pause",
            toggle_pause, "Continue",
            open_skills, "View Skills",
            return_prompt, "Return to Menu",
            NULL
        );
    }
    else
    {
        slog("Resuming game");
        _paused = 0;
        _menu = NULL;
    }
    toggle_music();
}

void load_editor()
{
    if (!is_editor_open()) window_editor();
}

int main(int argc, char * argv[])
{
    /*variable declarations*/
    const Uint8 * keys;
    Sprite* menu_bg;
    GFC_Vector2D mouse_pos;
    GFC_Rect ed_tile;
    
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

    /**
    _level = level_load("level/level1.json");
    //_level = level_load_bin("level/level1.bin");
    if (_level)
    {
        //level_save_bin(_level, "level/level1.bin");
    }

    _player = player_entity_get();
    if (!_player) _player = player_entity_new(gfc_vector2d(
        (float)_level->width * _level->tileDef->width / 2,
        (float)_level->height * _level->tileDef->height / 2)
    );
    */

    load_main_menu();
    menu_bg = gf2d_sprite_load_image("images/backgrounds/bg_mountain.png");

    /*main game loop*/
    while(!_done)
    {
        gfc_input_update(); // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        gf2d_mouse_update();
        gf2d_windows_update_all();

        if (_level && !is_editor_open())
        {
            // pausing
            if (gfc_input_key_pressed("q") && !_paused) toggle_pause(NULL);

            if (!_paused)
            {
                entity_manager_think_all();
                entity_manager_update_all();
                music_update();
            }
        }
        else
        {
            // main menu stuff
            music_update();
            if (gfc_input_key_pressed("e")) load_editor();
        }
        
        gf2d_graphics_clear_screen();// clears drawing buffers
        // all drawing should happen between clear_screen and next_frame
            //backgrounds drawn first
        if (_level && !is_editor_open())
        {
            if (!_paused)
            {
                _level = get_current_level();
                if (_level && _level->background) level_draw(_level);

                entity_manager_draw_all();
                if (_player && _player->_inuse) entity_draw(_player);

                hud_update(_player);
            }
            else if (_level->background)
                gf2d_sprite_draw_image(_level->background, gfc_vector2d(0, 0));
        }
        else if (!is_editor_open())
        {
            gf2d_sprite_draw_image(menu_bg, gfc_vector2d(0, 0));
        }
            
        //UI elements last
        gf2d_windows_draw_all();
        if (is_editor_open())
        {
            editor_draw_tiles();
            mouse_pos = gf2d_mouse_get_position();
            if (mouse_pos.x < 960 && mouse_pos.y >= 144)
            {
                //slog("%f %f", mouse_pos.x, mouse_pos.y);
                mouse_pos = gfc_vector2d(
                    32 * (int)(mouse_pos.x / 32),
                    32 * (int)(mouse_pos.y / 32 + .5) - 16
                );
                ed_tile = gfc_rect(mouse_pos.x, mouse_pos.y, 32, 32);
                gf2d_draw_rect(ed_tile, gfc_color8(128, 0, 255, 255));
                //slog("%f %f %f %f", ed_tile.x, ed_tile.y, ed_tile.w, ed_tile.h);
            }
        }

        gf2d_mouse_draw();

        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame

        if (_level && !is_editor_open())
        {
            if (_player && _player->_inuse &&
                _player->position.y + _player->bounds.y > _level->height * _level->tileDef->height)
            {
                player_kill("Player fell from a high place");
            }
            if (!_player || !_player->_inuse)
            {
                return_to_menu(NULL);
            }
        }
        
        /**
        if (keys[SDL_SCANCODE_ESCAPE] && _ex == NULL && !_paused)
        {
            exit_window(NULL);
            toggle_pause(NULL);
        }
        */

        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    slog("---==== END ====---");
    music_queue_free();
    gf2d_sprite_free(menu_bg);
    return 0;
}
/*eol@eof*/
