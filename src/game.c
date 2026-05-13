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
#include "skill_tree.h"
#include "cutscene.h"
#include "entity.h"
#include "player.h"
#include "monster.h"
#include "level.h"

static int _done = 0;
static int _paused = 0;
static int _alive = 0;
static Window* _menu = NULL;
static Window* _ex = NULL;
Level* _level;
Entity* _player;
Uint8 _l;

void toggle_pause(void* data);
void load_main_menu();

void open_skills(void* data)
{
    if (is_skill_tree_open()) return;
    window_skill_tree();
    gf2d_window_free(_menu);
    _menu = NULL;
}

void on_exit(void* data)
{
    _done = 1;
    _ex = NULL;
}

void on_cancel(void* data)
{
    _ex = NULL;
    if ((!_menu && _alive) || (!_level && _paused)) toggle_pause(NULL);
}

void load_level(void* data)
{
    const char* path;
    if (get_current_level()) level_free(get_current_level());
    gf2d_window_free(_menu);
    _menu = NULL;
    _ex = NULL;
    path = (const char*)(data);
    level_load(path, 1);
    if (!get_current_level())
    {
        level_load_bin(path, 1);
    }

    _player = player_entity_get();
    _alive = 1;
    _level = get_current_level();
    _done = 0;
}

void load_level1(void* data)
{
    load_level((void*)("level/level1.bin"));
    _l = 1;
}

void load_level2(void* data)
{
    load_level((void*)("level/level2.bin"));
    _l = 2;
}

void load_level3(void* data)
{
    load_level((void*)("level/level3.bin"));
    //load_level((void*)("level/boss3.bin"));
    _l = 3;
}

void open_select_window(void *data)
{
    Uint8 levels_beaten, free;
    Entity* player;
    free = (Uint8)(data);
    if (!free) gf2d_window_free(_menu); // ironic, I know
    _menu = NULL;
    _l = 0;
    player = player_entity_new(gfc_vector2d(0, 0));
    levels_beaten = level_cleared(player, 1) + 2 * level_cleared(player, 2) + 4 * level_cleared(player, 3);
    entity_free(player);
    _menu = window_select(load_level1, load_level2, load_level3, load_main_menu, (void *)levels_beaten);
    load_music_pair("audio/music/title_intro.mp3", "audio/music/title_loop.mp3", 1);
}

void delete_data(void* data)
{
    int i;
    wipe_data();
    open_select_window(data);
}

void overwrite_save(void* data)
{
    if (!player_load()) open_select_window(data);
    else _ex = window_yes_no("Warning: existing data found.",
        "Overwrite data and start new game?", delete_data, on_cancel, NULL);
}

void no_save(void* data)
{
    if (player_load()) open_select_window(data);
    else _ex = window_yes_no("No save data found.", "Create a new game?", open_select_window, on_cancel, NULL);
}

void exit_window(void* data)
{
    _ex = window_yes_no("Exit the game?", NULL, on_exit, on_cancel, NULL);
}

void load_main_menu()
{
    _menu = window_menu(
        "Menu",
        overwrite_save, "New Game",
        no_save, "Continue",
        exit_window, "Quit Game",
        NULL
    );
    load_music_pair("audio/music/title_intro.mp3", "audio/music/title_loop.mp3", 1);
}

void die(void* data)
{
    _alive = 0;
    _level = NULL;

    if (_menu) gf2d_window_free(_menu);
    _menu = NULL;
    _paused = 0;

    on_cancel(data);
    load_music_pair("audio/music/stop.mp3", "audio/music/stop.mp3", 1);
    gf2d_windows_play_sound("death");

    _menu = window_alert("You died!", "Click to return to the menu.", load_main_menu, NULL);
}

void you_win(void* data)
{
    char message[32];

    if (level_cleared(_player, _l)) sprintf(message, "Click to select another level.");
    else sprintf(message, "You got %i skill points!", 2 * _l);
    beat_level(_player, _l);

    entity_free(_player);
    _alive = 0;
    _level = NULL;

    if (_menu) gf2d_window_free(_menu);
    _menu = NULL;
    _paused = 0;

    on_cancel(data);
    load_music_pair("audio/music/stop.mp3", "audio/music/stop.mp3", 1);
    gf2d_windows_play_sound("win");

    _menu = window_alert("Level cleared!", message, open_select_window, (void *)(1));
}

void return_to_menu(void* data)
{
    if (_alive)
    {
        entity_free(_player);
        _alive = 0;
        _level = NULL;
    }
    if (is_cutscene()) cutscene_free();
    gf2d_window_free(_menu);
    on_cancel(data);
    load_main_menu();
}

void return_prompt(void* data)
{
    _ex = window_yes_no("Return to the menu?", "(Player data will be saved.)", return_to_menu, on_cancel, NULL);
}

void pause_menu(void* data)
{
    _menu = window_menu(
        "Pause",
        toggle_pause, "Continue",
        open_skills, "View Skills",
        return_prompt, "Return to Menu",
        NULL
    );
}

void toggle_pause(void* data)
{
    if (_paused)
    {
        _paused = 0;
        if (_menu)
        {
            gf2d_window_free(_menu);
            _menu = NULL;
        }
    }
    else
    {
        if (_ex) return;
        _paused = 1;
        pause_menu(data);
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
    int e, s, c;
    
    /*program initialization*/
    init_logger("gf2d.log",0);
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
        e = is_editor_open();
        s = is_skill_tree_open();
        c = is_cutscene();

        if (_level && !e)
        {
            // pausing
            if (gfc_input_key_pressed("q") && !_paused && !_ex && !c)
            {
                toggle_pause(NULL);
                gf2d_windows_play_sound("cancel");
            }

            if (!_paused)
            {
                entity_manager_think_all();
                entity_manager_update_all();
                if (!get_current_level() && _player && _player->health)
                {
                    you_win(NULL);
                }
                if (c)
                {
                    camera_center_on(_player->position);
                    cutscene_update();
                }
                music_update();
            }
            else
            {
                if (!s && !_menu) pause_menu(NULL);
            }
        }
        else
        {
            // main menu stuff
            music_update();
            if (gfc_input_key_pressed("e"))
            {
                load_editor();
                e = is_editor_open();
            }
        }
        
        gf2d_graphics_clear_screen();// clears drawing buffers
        // all drawing should happen between clear_screen and next_frame
            //backgrounds drawn first
        if (_level && !e)
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
        else if (!e)
        {
            gf2d_sprite_draw_image(menu_bg, gfc_vector2d(0, 0));
        }
        
        //UI elements last
        gf2d_windows_draw_all();
        if (e)
        {
            editor_draw_tiles();
            mouse_pos = gf2d_mouse_get_position();
            if (mouse_pos.x < 960 && mouse_pos.y >= 144)
            {
                mouse_pos = gfc_vector2d(
                    32 * (int)(mouse_pos.x / 32),
                    32 * (int)(mouse_pos.y / 32 + .5) - 16
                );
                ed_tile = gfc_rect(mouse_pos.x, mouse_pos.y, 32, 32);
                gf2d_draw_rect(ed_tile, gfc_color8(128, 0, 255, 255));
            }
            if (e >= 2) redraw_win2();
        }
        if (c) cutscene_draw_portrait();

        gf2d_mouse_draw();

        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame

        if (_level && !e)
        {
            if (_player && _player->_inuse &&
                _player->position.y + _player->bounds.y > _level->height * _level->tileDef->height)
            {
                player_kill();
            }
            if (!_player || !_player->_inuse)
            {
                die(NULL);
            }
        }
    }
    gf2d_sprite_free(menu_bg);
    return 0;
}
/*eol@eof*/
