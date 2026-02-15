#include <SDL.h>
#include "simple_logger.h"

#include "gfc_audio.h"
#include "gfc_input.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"

#include "camera.h"
#include "entity.h"
#include "player.h"
#include "monster.h"
#include "level.h"

int main(int argc, char * argv[])
{
    /*variable declarations*/
    int done = 0;
    int i;
    const Uint8 * keys;
    Level *level;
    
    int mx,my;
    float mf = 0;
    Sprite *mouse;
    GFC_Color mouseGFC_Color = gfc_color8(0,204,255,200);

    GFC_Sound *bgm;
    
    /*program initializtion*/
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
    camera_set_dimensions(gfc_vector2d(1200, 720));
    gfc_input_init("config/input.cfg");
    gf2d_graphics_set_frame_delay(16);
    gf2d_sprite_init(1024);
    entity_manager_init(1024);
    gfc_audio_init(32, 4, 1, 4, true, false);
    SDL_ShowCursor(SDL_DISABLE);
    
    /*demo setup*/
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    bgm = gfc_sound_load("audio/song_test.wav", -1, -1);

    gfc_sound_play(bgm, -1, -1, -1, -1);

    slog("press [escape] to quit");

    level = level_load("level/testlevel.json");
    if (level)
    {
        level_add_border(level, 4);
    }

    Entity* player = player_entity_new(gfc_vector2d(
        (float) level->width * level->tileDef->width / 2,
        (float) level->height * level->tileDef->height / 2));
    i = 0;

    /*main game loop*/
    while(!done)
    {
        gfc_input_update(); // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        SDL_GetMouseState(&mx,&my);
        mf += 0.1f;
        if (mf >= 16.0)mf = 0;

        // spawn a new monster every second or so (or if e pressed)
        i++;
        if (gfc_input_key_pressed("e") || i == 100) {
            if (i == 100) i = 0;
            float x = (float) (1 + gfc_crandom()) * level->width * level->tileDef->width / 2;
            float y = (float) (1 + gfc_crandom()) * level->height * level->tileDef->height / 2;
            monster_new(gfc_vector2d(x, y));
        }

        // randomly delete a monster every time q is pressed
        if (gfc_input_key_pressed("q"))
        {
            entity_manager_kill_random();
        }

        entity_manager_think_all();
        entity_manager_update_all();
        
        gf2d_graphics_clear_screen();// clears drawing buffers
        // all drawing should happen between clear_screen and next_frame
            //backgrounds drawn first
            level_draw(level);

            entity_manager_draw_all();
            entity_draw(player);
            
            //UI elements last
            gf2d_sprite_draw(
                mouse,
                gfc_vector2d((float) mx, (float) my),
                NULL,
                NULL,
                NULL,
                NULL,
                &mouseGFC_Color,
                (int)mf);

        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
        
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    gfc_sound_clear_all();
    slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
