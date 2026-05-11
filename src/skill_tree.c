#include "simple_logger.h"

#include "gfc_callbacks.h"

#include "gf2d_elements.h"
#include "gf2d_windows.h"
#include "gf2d_windows_common.h"

#include "skill_tree.h"
#include "player.h"

Window* _win_st;
Entity* _player_st;

void purchase_skill(void* data)
{
    short unsigned int skillInfo;
    Uint8 skillId, skillCost;
    skillInfo = (short unsigned int*)data;
    skillId = skillInfo >> 8;
    skillCost = skillInfo - (skillId << 8);
    if (skillCost > get_skill_points(_player_st))
    {
        window_alert("Insufficient SP", "You don't have enough SP to purchase this skill.", NULL, NULL);
        gf2d_windows_play_sound("notify");
        return;
    }

    spend_skill_points(_player_st, skillCost);
    grant_skill(_player_st, skillId);
    gf2d_window_free(_win_st);
    _win_st = window_skill_tree(); // convoluted, but should set color properly
    window_alert("Skill purchased", "You have unlocked this skill.", NULL, NULL);
    gf2d_windows_play_sound("unlock");
}

void skill_purchase_prompt(Uint8 skillId, const char* skillName, const char* skillDesc, Uint8 skillCost)
{
    GFC_TextLine header;
    short unsigned int skillInfo;
    skillInfo = (skillId << 8) + skillCost;
    sprintf(header, "Acquire %s for %i SP?", skillName, skillCost);
    window_yes_no(header, skillDesc, purchase_skill, NULL, (void*)skillInfo);
}

void skill_already_owned(const char* skillName)
{
    GFC_TextLine desc;
    sprintf(desc, "You already purchased %s", skillName);
    window_alert("Skill already unlocked", desc, NULL, NULL);
    gf2d_windows_play_sound("notify");
}

void missing_prereq(const char* skillName, const char* prereq)
{
    GFC_TextLine desc;
    sprintf(desc, "Cannot purchase %s without unlocking %s", skillName, prereq);
    window_alert("Missing prerequisite", desc, NULL, NULL);
    gf2d_windows_play_sound("notify");
}

void check_skill_owned(Uint8 skillId, const char* skillName, const char* skillDesc, Uint8 skillCost)
{
    if (check_skill(_player_st, skillId)) skill_already_owned(skillName);
    else skill_purchase_prompt(skillId, skillName, skillDesc, skillCost);
}

void acquire_power1(void* data)
{
    check_skill_owned(SO_Power1, "Power 1", "Increases your damage by 2.", 1);
}

void acquire_power2(void* data)
{
    if (!check_skill(_player_st, SO_Power1)) missing_prereq("Power 2", "Power 1");
    else check_skill_owned(SO_Power2, "Power 2", "Further increases your damage by 2.", 2);
}

void acquire_speed1(void* data)
{
    check_skill_owned(SO_Speed1, "Speed 1", "Increases your speed by 10%.", 1);
}

void acquire_speed2(void* data)
{
    if (!check_skill(_player_st, SO_Speed1)) missing_prereq("Speed 2", "Speed 1");
    else check_skill_owned(SO_Speed2, "Speed 2", "Further increases your speed by 10%.", 2);
}

void acquire_double_jump(void* data)
{
    if (!check_skill(_player_st, SO_Speed2) && !check_skill(_player_st, SO_MaxHealth))
        missing_prereq("Double Jump", "Speed 2 or Health Boost");
    else check_skill_owned(SO_DoubleJump, "Double Jump", "Permanently unlocks the double jump.", 2);
}

void acquire_hover(void* data)
{
    if (!check_skill(_player_st, SO_Power2) && !check_skill(_player_st, SO_MaxHealth))
        missing_prereq("Hover", "Power 2 or Health Boost");
    else check_skill_owned(SO_Hover, "Hover", "Permanently unlocks the hover jump.", 2);
}

void acquire_health_boost(void* data)
{
    if (!check_skill(_player_st, SO_Power1) && !check_skill(_player_st, SO_Speed1))
        missing_prereq("Health Boost", "Power 1 or Speed 1");
    else check_skill_owned(SO_MaxHealth, "Health Boost", "Doubles your maximum HP.", 2);
}

Uint8 is_skill_tree_open()
{
    if (!_win_st) return 0;
    return 1;
}

void exit_skill_tree(void* data)
{
    gf2d_window_free(_win_st);
    _win_st = NULL;
}

int skill_tree_free(Window* win)
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

int skill_tree_update(Window* win, GFC_List* updateList)
{
    int i, count;
    Element* e;
    GFC_List* callbacks;
    GFC_Callback* callback;
    if (!win)return 0;
    if (!updateList)return 0;

    callbacks = (GFC_List*)win->data;
    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList, i);
        if (!e)continue;

        switch (e->index)
        {
            case 2:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 0);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
            case 10:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 1);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
            case 11:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 2);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
            case 12:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 3);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
            case 13:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 4);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
            case 14:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 5);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
            case 15:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 6);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
            case 16:
                callback = (GFC_Callback*)gfc_list_get_nth(callbacks, 7);
                if (callback)
                {
                    gfc_callback_call(callback);
                }
                return 1;
        }

        return 1;
    }
    return 1;
}

Window* window_skill_tree()
{
    GFC_List* callbacks;
    char* buf[5];
    _win_st = gf2d_window_load("menus/skill_tree_menu.json");
    if (!_win_st)
    {
        return NULL;
    }

    _player_st = player_entity_get();
    if (!_player_st)
    {
        window_alert("Error", "No player found", NULL, NULL);
        return NULL;
    }
    sprintf(buf, "%i SP", get_skill_points(_player_st));
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(_win_st, 1), buf);
    for (int i = 0; i < 7; i++)
    {
        if (!check_skill(_player_st, 1 << i))
        {
            gf2d_element_set_color(gf2d_window_get_element_by_id(_win_st, 20 + i), GFC_COLOR_WHITE);
        }
    }

    _win_st->update = skill_tree_update;
    _win_st->free_data = skill_tree_free;
    callbacks = gfc_list_new();
    gfc_list_append(callbacks, gfc_callback_new(exit_skill_tree, NULL)); // for quit button
    gfc_list_append(callbacks, gfc_callback_new(acquire_power1, NULL)); // for power 1
    gfc_list_append(callbacks, gfc_callback_new(acquire_power2, NULL)); // for power 2
    gfc_list_append(callbacks, gfc_callback_new(acquire_speed1, NULL)); // for speed 1
    gfc_list_append(callbacks, gfc_callback_new(acquire_speed2, NULL)); // for speed 2
    gfc_list_append(callbacks, gfc_callback_new(acquire_double_jump, NULL)); // for double jump
    gfc_list_append(callbacks, gfc_callback_new(acquire_hover, NULL)); // for hover
    gfc_list_append(callbacks, gfc_callback_new(acquire_health_boost, NULL)); // for health boost
    _win_st->data = callbacks;

    return _win_st;
}