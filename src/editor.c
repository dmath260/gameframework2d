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
Level* _level;

Uint8 is_editor_open()
{
    if (!_win) return 0;
    return 1;
}

void exit_editor(void* data)
{
    load_music_pair("audio/music/title_intro.mp3", "audio/music/title_loop.mp3");
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
    _ent_id = -1;
    increment_entity(NULL);
    _level = level_load("level/testlevel.json");
    gf2d_element_actor_set_image(gf2d_window_get_element_by_id(_win, 17), _level->tileDef->sheet);
    _tile_id = 1;
    load_music_pair("audio/music/editor0_intro.mp3", "audio/music/editor0_loop.mp3");
    return _win;
}