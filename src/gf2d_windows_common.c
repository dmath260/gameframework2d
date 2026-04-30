#include "simple_logger.h"

#include "gfc_list.h"
#include "gfc_input.h"
#include "gfc_callbacks.h"

#include "gf2d_mouse.h"
#include "gf2d_elements.h"
#include "gf2d_element_label.h"
#include "gf2d_element_entry.h"

#include "gf2d_windows_common.h"

int yes_no_free(Window *win)
{
    GFC_List *list;
    int count,i;
    GFC_Callback *callback;

    if (!win)return 0;
    if (!win->data)return 0;

    list = (GFC_List*)win->data;
    if (list)
    {
    count = gfc_list_get_count(list);

        for (i = 0; i < count; i++)
        {
            callback = (GFC_Callback*)gfc_list_get_nth(list,i);
            if (callback)
            {
                gfc_callback_free(callback);
            }
        }

        gfc_list_delete(list);
    }
    return 0;
}

int yes_no_update(Window *win, GFC_List *updateList)
{
    int i,count;
    Element *e;
    Element *focus;
    GFC_List *callbacks;
    GFC_Callback *callback;
    if (!win)return 0;
    if (!updateList)return 0;
    
    if ((gf2d_mouse_hidden())&&(gfc_input_command_pressed("nextelement")))
    {
        gf2d_window_next_focus(win);
        return 1;
    }

    callbacks = (GFC_List*)win->data;
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if ((strcmp(e->name,"item_right")==0)||(strcmp(e->name,"item_left")==0))
        {
            focus = gf2d_window_get_element_by_focus(win);
            if ((!focus)||(focus->index == 52))
            {
                gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,51));
            }
            else
            {
                gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,52));
            }
            return 1;
        }
        switch(e->index)
        {
            case 51:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks,0);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                gf2d_window_free(win);
                return 1;
                break;
            case 52:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks,1);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                gf2d_window_free(win);
                return 1;
        }
    }
    return 1;
}

Window *window_yes_no(char *text,char *text2,void(*onYes)(void *),void(*onNo)(void *),void *data)
{
    Window* win = {0};
    GFC_List *callbacks;
    if (text2) win = gf2d_window_load("menus/yes_no_window_2.json");
    else win = gf2d_window_load("menus/yes_no_window.json");
    if (!win)
    {
        slog("failed to load yes/no window");
        return NULL;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win,1),text);
    if (text2) gf2d_element_label_set_text(gf2d_window_get_element_by_id(win,2),text2);
    if (gf2d_mouse_hidden())gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,51));
    win->update = yes_no_update;
    win->free_data = yes_no_free;
    callbacks = gfc_list_new();
    if (onYes)
    {
        gfc_list_append(callbacks,gfc_callback_new(onYes,data));
    }
    if (onNo)
    {
        gfc_list_append(callbacks,gfc_callback_new(onNo,data));
    }
    win->data = callbacks;
    return win;
}

Window* window_a_b(char* text, char* text2, char* textA, char* textB, void(*onA)(void*), void(*onB)(void*), void* data)
{
    Window* win = { 0 };
    GFC_List* callbacks;
    if (text2) win = gf2d_window_load("menus/yes_no_window_2.json");
    else win = gf2d_window_load("menus/yes_no_window.json");
    if (!win)
    {
        slog("failed to load yes/no window");
        return NULL;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win, 1), text);
    if (text2) gf2d_element_label_set_text(gf2d_window_get_element_by_id(win, 2), text2);
    if (textA) gf2d_element_label_set_text(gf2d_window_get_element_by_id(win, 61), textA);
    if (textB) gf2d_element_label_set_text(gf2d_window_get_element_by_id(win, 62), textB);
    if (gf2d_mouse_hidden())gf2d_window_set_focus_to(win, gf2d_window_get_element_by_id(win, 51));
    win->update = yes_no_update;
    win->free_data = yes_no_free;
    callbacks = gfc_list_new();
    if (onA)
    {
        gfc_list_append(callbacks, gfc_callback_new(onA, data));
    }
    if (onB)
    {
        gfc_list_append(callbacks, gfc_callback_new(onB, data));
    }
    win->data = callbacks;
    return win;
}

int menu_free(Window* win)
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

int menu_update(Window* win, GFC_List* updateList)
{
    int i, count;
    Element* e;
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
        switch (e->index)
        {
        case 11:
            callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 0);
            if (callback)
            {
                gfc_callback_call(callback);
            }
            return 1;
            break;
        case 13:
            callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 1);
            if (callback)
            {
                gfc_callback_call(callback);
            }
            return 1;
            break;
        case 15:
            callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 2);
            if (callback)
            {
                gfc_callback_call(callback);
            }
            return 1;
        }
    }
    return 1;
}

Window* window_menu(
    char* title,
    void(*onButton1)(void*),
    char* button1,
    void(*onButton2)(void*),
    char* button2,
    void(*onButton3)(void*),
    char* button3,
    void *data
)
{
    Window* win;
    GFC_List* callbacks;
    win = gf2d_window_load("menus/main_pause_menu.json");
    if (!win)
    {
        slog("failed to load main/pause menu");
        return NULL;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win, 1), title);
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win, 12), button1);
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win, 14), button2);
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win, 16), button3);
    if (gf2d_mouse_hidden())gf2d_window_set_focus_to(win, gf2d_window_get_element_by_id(win, 11));
    win->update = menu_update;
    win->free_data = menu_free;
    callbacks = gfc_list_new();
    if (onButton1)
    {
        gfc_list_append(callbacks, gfc_callback_new(onButton1, data));
    }
    if (onButton2)
    {
        gfc_list_append(callbacks, gfc_callback_new(onButton2, data));
    }
    if (onButton3)
    {
        gfc_list_append(callbacks, gfc_callback_new(onButton3, data));
    }
    win->data = callbacks;
    return win;
}


int ok_update(Window *win, GFC_List *updateList)
{
    int i,count;
    Element *e;
    GFC_List *callbacks;
    GFC_Callback *callback;
    if (!win)return 0;
    if (!updateList)return 0;
    callbacks = (GFC_List*)win->data;
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        switch(e->index)
        {
            case 51:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks,0);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                gf2d_window_free(win);
                return 1;
                break;
        }
    }
    return 0;
}

int alert_update(Window *win, GFC_List *updateList)
{
    int i,count;
    Element *e;
    GFC_List *callbacks;
    GFC_Callback *callback;
    if (!win)return 0;
    callbacks = (GFC_List*)win->data;
    if (gf2d_mouse_button_pressed(0))
    {
        if (callbacks)
        {
            callback = (GFC_Callback*)gfc_list_get_nth(callbacks,0);
            if (callback)
            {
                gfc_callback_call(callback);
            }
        }
        gf2d_window_free(win);
        return 1;
    }
    if (!updateList)return 0;
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList,i);
        if (!e)continue;
        if (strcmp(e->name,"ok")==0)
        {
            callback = (GFC_Callback*)gfc_list_get_nth(callbacks,0);
            if (callback)
            {
                gfc_callback_call(callback);
            }
            gf2d_window_free(win);
        }
    }
    return 0;
}

Window *window_alert(char *title, char *text, void(*onOK)(void *),void *okData)
{
    Window *win;
    GFC_List *callbacks;
    win = gf2d_window_load("menus/alert_menu.json");
    if (!win)
    {
        slog("failed to load alert window");
        return NULL;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"title"),title);
    gf2d_element_label_set_text(gf2d_window_get_element_by_name(win,"text"),text);
    win->update = alert_update;
    win->free_data = yes_no_free;
    if (onOK)
    {
        callbacks = gfc_list_new();
        gfc_list_append(callbacks,gfc_callback_new(onOK,okData));
        win->data = callbacks;
    }
    return win;
}

Window *window_dialog(char *title, char *text, void(*onOK)(void *),void *okData)
{
    Window *win;
    GFC_List *callbacks;
    win = gf2d_window_load("menus/dialog.json");
    if (!win)
    {
        slog("failed to load alert window");
        return NULL;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win,1),title);
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win,2),text);
    win->update = alert_update;
    win->free_data = yes_no_free;
    if (onOK)
    {
        callbacks = gfc_list_new();
        gfc_list_append(callbacks,gfc_callback_new(onOK,okData));
        win->data = callbacks;
    }
    return win;
}


Window *window_text_entry(char *question, char *defaultText,void *callbackData, size_t length, void(*onOk)(void *),void(*onCancel)(void *))
{
    Window *win;
    GFC_List *callbacks;
    win = gf2d_window_load("menus/text_entry_window.json");
    if (!win)
    {
        slog("failed to load text entry window");
        return NULL;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win,1),question);
    gf2d_element_entry_set_text_pointer(gf2d_window_get_element_by_id(win,2),defaultText,length);
    gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,2));
    win->update = yes_no_update;
    win->free_data = yes_no_free;
    callbacks = gfc_list_new();
    if (onOk)
    {
        gfc_list_append(callbacks,gfc_callback_new(onOk,callbackData));
    }
    if (onCancel)
    {
        gfc_list_append(callbacks,gfc_callback_new(onCancel,callbackData));
    }
    win->data = callbacks;
    return win;
}

Window *window_key_value(char *question, char *defaultKey,char *defaultValue,void *callbackData, size_t keyLength,size_t valueLength, void(*onOk)(void *),void(*onCancel)(void *))
{
    Window *win;
    GFC_List *callbacks;
    win = gf2d_window_load("menus/key_value.json");
    if (!win)
    {
        slog("failed to load key_value window");
        return NULL;
    }
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win,1),question);
    gf2d_element_entry_set_text_pointer(gf2d_window_get_element_by_id(win,2),defaultKey,keyLength);
    gf2d_window_set_focus_to(win,gf2d_window_get_element_by_id(win,2));
    gf2d_element_entry_set_text_pointer(gf2d_window_get_element_by_id(win,3),defaultValue,valueLength);
    win->update = yes_no_update;
    win->free_data = yes_no_free;
    callbacks = gfc_list_new();
    if (onOk)
    {
        gfc_list_append(callbacks,gfc_callback_new(onOk,callbackData));
    }
    if (onCancel)
    {
        gfc_list_append(callbacks,gfc_callback_new(onCancel,callbackData));
    }
    win->data = callbacks;
    return win;
}

/*eol@eof*/
