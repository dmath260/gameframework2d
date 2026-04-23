#include "simple_logger.h"

#include "gfc_callbacks.h"
#include "gfc_input.h"

#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_label.h"
#include "gf2d_element_entry.h"
#include "gf2d_element_actor.h"
#include "gf2d_windows_common.h"

#include "editor.h"
#include "audio.h"
#include "entity.h"
#include "level.h"

Window* _win;
Uint8 _ent_id;
Uint8 _tile_id;
Uint8 _held_ent;
Uint8 _hotkey_mode;
Level* _level;
const char* _level_name;
GFC_Vector2D _camera;

Uint8 is_editor_open()
{
    if (!_win) return 0;
    return 1;
}

int get_index_at_mouse_pos()
{
    GFC_Vector2D mouse_pos;
    mouse_pos = gf2d_mouse_get_position();
    if (mouse_pos.x >= 960 || mouse_pos.y < 144) return -1;
    mouse_pos = gfc_vector2d(
        (int)(mouse_pos.x / 32 + _camera.x),
        (int)(mouse_pos.y / 32 + .5 + _camera.y) - 5
    );
    return level_get_tile_index(_level, mouse_pos.x, mouse_pos.y);
}

void load(void* data)
{
    const char* str;
    Level* level;
    str = (const char*)data;
    if (!str)
    {
        slog("Invalid string");
        return;
    }
    level = level_load(str, 0);
    if (!level)
    {
        slog("Failed to load level %s", str);
        return;
    }
    _level = level;
    _level_name = str;
    set_current_level(NULL);
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(_win, 15), _level_name);
    gf2d_element_actor_set_image(gf2d_window_get_element_by_id(_win, 10), _level->background);
}

void exit_editor(void* data)
{
    load_music_pair("audio/music/title_intro.mp3", "audio/music/title_loop.mp3");
    if (_level) level_free(_level);
    _level = NULL;
    gf2d_window_free(_win);
    _win = NULL;
}

void increment_tile(void* data)
{
    Element* tile_icon;
    ActorElement* tile_element;
    tile_icon = gf2d_window_get_element_by_id(_win, 17);
    tile_element = (ActorElement*)tile_icon->data;
    if (!tile_element) return;
    if (_tile_id == 73)
    {
        _tile_id = 0;
        tile_element->frame = 73;
    }
    else
    {
        _tile_id++;
        tile_element->frame = _tile_id - 1;
    }
}

void decrement_tile(void* data)
{
    Element* tile_icon;
    ActorElement* tile_element;
    tile_icon = gf2d_window_get_element_by_id(_win, 17);
    tile_element = (ActorElement*)tile_icon->data;
    if (!tile_element) return;
    if (_tile_id == 0)
    {
        _tile_id = 73;
        tile_element->frame = 72;
    }
    else if (_tile_id == 1)
    {
        _tile_id = 0;
        tile_element->frame = 73;
    }
    else
    {
        _tile_id--;
        tile_element->frame = _tile_id - 1;
    }
}

void replace_tile(Uint8 tid)
{
    if (tid > 73) return;
    int i;
    i = get_index_at_mouse_pos();
    if (i >= 0)
    {
        _level->tileMap[i] = tid;
        if (tid != 0 && _level->entityMap[i]) _level->entityMap[i] = 0;
    }
}

void increment_entity(void* data)
{
    Uint8 id = _ent_id;
    EntityEntry* ent;
    while (1)
    {
        _ent_id++;
        if (_ent_id == id)
        {
            slog("Could not find any other entities.");
            return;
        }
        ent = get_entity_data_at_id(_ent_id);
        if (ent) break;
    }
    gf2d_element_actor_set_image(gf2d_window_get_element_by_id(_win, 19), ent->icon);
}

void decrement_entity(void* data)
{
    Uint8 id = _ent_id;
    EntityEntry* ent;
    while (1)
    {
        _ent_id--;
        if (_ent_id == id)
        {
            slog("Could not find any other entities.");
            return;
        }
        ent = get_entity_data_at_id(_ent_id);
        if (ent) break;
    }
    gf2d_element_actor_set_image(gf2d_window_get_element_by_id(_win, 19), ent->icon);
}

void replace_entity(Uint8 eid)
{
    int i;
    i = get_index_at_mouse_pos();
    if (i >= 0)
    {
        _level->entityMap[i] = eid;
        if (eid != 0 && _level->tileMap[i]) _level->tileMap[i] = 0;
    }
}

void pan_camera(Uint8 dir)
{
    switch (dir)
    {
        case 0:
            if (_camera.y == 0) slog("Cannot move farther up.");
            else _camera.y--;
            break;
        case 1:
            if (_camera.y == _level->height - 18) slog("Cannot move farther down.");
            else _camera.y++;
            break;
        case 2:
            if (_camera.x == 0) slog("Cannot move left.");
            else _camera.x--;
            break;
        case 3:
            if (_camera.x == _level->height - 30) slog("Cannot move farther right.");
            else _camera.x++;
            break;
        default:
            slog("Direction not recognized.");
    }
}

void editor_draw_tiles()
{
    int i, j, k, f, e, p;
    EntityEntry* ent;
    GFC_Vector2D scale = {0};
    scale = gfc_vector2d(0.5, 0.5);
    for (i = 0; i < 18; i++)
    {
        for (j = 0; j < 30; j++)
        {
            k = level_get_tile_index(_level, j + _camera.x, i + _camera.y);
            f = _level->tileMap[k] - 1;
            if (f >= 0) gf2d_sprite_draw(
                _level->tileDef->sheet,
                gfc_vector2d(32 * j, 32 * i + 144),
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                f
            );
            e = _level->entityMap[k];
            if (e > 0 && e <= 255) {
                ent = get_entity_data_at_id(e);
                if (ent)
                {
                    gf2d_sprite_draw(
                        ent->icon,
                        gfc_vector2d(32 * j, 32 * i + 144),
                        &scale,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        0
                    );
                }
            }
            p = get_index_at_mouse_pos();
            if (k == p && p >= 0) {
                ent = get_entity_data_at_id(_held_ent);
                if (ent)
                {
                    gf2d_sprite_draw(
                        ent->icon,
                        gfc_vector2d(32 * j, 32 * i + 144),
                        &scale,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        0
                    );
                }
            }
        }
    }
}

int editor_free(Window* win)
{
    GFC_List* list;
    int count, i;
    GFC_Callback* callback;

    if (!win)return 0;
    if (!win->data)return 0;

    list = (GFC_List*)win->data;
    if (list)
    {
        count = gfc_list_get_count(list);

        for (i = 0; i < count; i++)
        {
            callback = (GFC_Callback*)gfc_list_get_nth(list, i);
            if (callback)
            {
                gfc_callback_free(callback);
            }
        }

        gfc_list_delete(list);
    }
    return 0;
}

int editor_update(Window* win, GFC_List* updateList)
{
    int i, count;
    Element* e;
    Element* focus;
    GFC_List* callbacks;
    GFC_Callback* callback;
    if (!win)return 0;
    if (!updateList)return 0;

    if ((gf2d_mouse_hidden()) && (gfc_input_command_pressed("nextelement")))
    {
        gf2d_window_next_focus(win);
        return 1;
    }

    if (gfc_input_key_pressed("/"))
    {
        _hotkey_mode = (_hotkey_mode + 1) % 2;
        gf2d_windows_play_sound("confirm");
    }
    else if (gfc_input_key_pressed(","))
    {
        if (_hotkey_mode) decrement_entity(NULL);
        else decrement_tile(NULL);
        gf2d_windows_play_sound("confirm");
    }
    else if (gfc_input_key_pressed("."))
    {
        if (_hotkey_mode) increment_entity(NULL);
        else increment_tile(NULL);
        gf2d_windows_play_sound("confirm");
    }

    if (gfc_input_key_pressed("UP")) pan_camera(0);
    else if (gfc_input_key_pressed("DOWN")) pan_camera(1);
    else if (gfc_input_key_pressed("LEFT")) pan_camera(2);
    else if (gfc_input_key_pressed("RIGHT")) pan_camera(3);

    if (gf2d_mouse_button_held(0) &&
        (gfc_input_key_held("LSHIFT") || gfc_input_key_held("RSHIFT")))
        replace_tile(0);
    else if (gf2d_mouse_button_held(0)) replace_tile(_tile_id);
    else if (gfc_input_key_pressed("m")) replace_entity(_ent_id);
    else if (gfc_input_key_pressed("n")) replace_entity(0);
    else if (gfc_input_key_pressed(" "))
    {
        i = get_index_at_mouse_pos();
        if (i >= 0) {
            _held_ent = _level->entityMap[i];
            replace_entity(0);
        }
    }
    else if (gfc_input_key_released(" ") && _held_ent)
    {
        replace_entity(_held_ent);
        _held_ent = 0;
    }

    callbacks = (GFC_List*)win->data;
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList, i);
        if (!e)continue;
        if ((strcmp(e->name, "item_right") == 0) || (strcmp(e->name, "item_left") == 0))
        {
            // Update this later
            focus = gf2d_window_get_element_by_focus(win);
            if ((!focus) || (focus->index == 52))
            {
                gf2d_window_set_focus_to(win, gf2d_window_get_element_by_id(win, 51));
            }
            else
            {
                gf2d_window_set_focus_to(win, gf2d_window_get_element_by_id(win, 52));
            }
            return 1;
        }
        switch (e->index)
        {
            case 1:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 0);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
                break;
            case 2:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 1);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
                break;
            case 3:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 2);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
                break;
            case 4:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 3);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
                break;
            case 5:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 4);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
                break;
            case 6:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 5);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
                break;
            case 7:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 6);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
                break;
            case 8:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 7);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
                break;
        }
    }
    return 1;
}

Window* window_editor()
{
    GFC_List* callbacks;
    _win = gf2d_window_load("menus/editor_menu.json");
    if (!_win)
    {
        slog("failed to load editor window");
        return NULL;
    }

    _win->update = editor_update;
    _win->free_data = editor_free;
    callbacks = gfc_list_new();
    gfc_list_append(callbacks, gfc_callback_new(NULL, NULL)); // for load button
    gfc_list_append(callbacks, gfc_callback_new(NULL, NULL)); // for save button
    gfc_list_append(callbacks, gfc_callback_new(NULL, NULL)); // for settings button
    gfc_list_append(callbacks, gfc_callback_new(exit_editor, NULL)); // for quit button
    gfc_list_append(callbacks, gfc_callback_new(decrement_tile, NULL)); // for left tile button
    gfc_list_append(callbacks, gfc_callback_new(increment_tile, NULL)); // for right tile button
    gfc_list_append(callbacks, gfc_callback_new(decrement_entity, NULL)); // for left entity button
    gfc_list_append(callbacks, gfc_callback_new(increment_entity, NULL)); // for right entity button
    _win->data = callbacks;

    _tile_id = 1;
    _ent_id = -1;
    increment_entity(NULL);
    load((void *)("level/testlevel.json"));
    gf2d_element_actor_set_image(gf2d_window_get_element_by_id(_win, 17), _level->tileDef->sheet);
    _hotkey_mode = 0;
    _camera = gfc_vector2d(0, 0);
    load_music_pair("audio/music/editor0_intro.mp3", "audio/music/editor0_loop.mp3");
    return _win;
}