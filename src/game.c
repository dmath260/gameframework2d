#include <SDL.h>
#include "simple_logger.h"

#include "gfc_input.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"

#include "camera.h"
#include "audio.h"
#include "entity.h"
#include "player.h"
#include "monster.h"
#include "level.h"

int main(int argc, char * argv[])
{
    /*variable declarations*/
    int done = 0, paused = 0;
    //int i;
    const Uint8 * keys;
    Level *level;
    
    int mx,my;
    float mf = 0;
    Sprite *mouse;
    GFC_Color mouseGFC_Color = gfc_color8(0,204,255,200);
    
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
    audio_init(32, 4, 1, 4, true, false);
    SDL_ShowCursor(SDL_DISABLE);
    
    /*demo setup*/
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);

    // NOTE: BE CONSISTENT ABOUT MUSIC SAMPLE RATE!
    // Either use all 44.1 kHz, or use all 48 kHz. Nothing else will work.
    //enqueue_music("audio/silence.mp3", 0); // buffer for music queue
    //enqueue_music("audio/boss1_intro.mp3", 0);
    //enqueue_music("audio/boss1_loop.mp3", -1);

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

    //i = 0;
    //float x, y;
    //MonsterTypes t;
    //ItemTypes j;

    /*main game loop*/
    while(!done)
    {
        gfc_input_update(); // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        SDL_GetMouseState(&mx,&my);
        mf += 0.1f;
        if (mf >= 16.0)mf = 0;

        // pausing
        if (gfc_input_key_pressed("q"))
        {
            if (!paused)
            {
                paused = 1;
            }
            else
            {
                paused = 0;
            }
            toggle_music();
        }

        // spawn a new monster every second or so (or if e pressed)
        /*
        i++;
        if (gfc_input_key_pressed("e") || i == 500) {
            if (i == 500) i = 0;
            x = (float) (1 + gfc_crandom()) * level->size.x / 2;
            y = (float) (1 + gfc_crandom()) * level->size.y / 2;
            t = (MonsterTypes)((1 + gfc_crandom()) * MT_MAX / 2);
            //t = MT_Grunt;
            j = (ItemTypes)((1 + gfc_crandom()) * IT_MAX / 2);
            //j = IT_Invincible;
            monster_new(gfc_vector2d(x, y), t, j);
        }
        */

        if (!paused)
        {
            entity_manager_think_all();
            entity_manager_update_all();
            music_update(); // move outside of if statement to continue music when paused
        }
        
        gf2d_graphics_clear_screen();// clears drawing buffers
        // all drawing should happen between clear_screen and next_frame
            //backgrounds drawn first
            level = get_current_level();
            if (level && level->background) level_draw(level);
            
            entity_manager_draw_all();
            if (player && player->_inuse) entity_draw(player);
            
            //UI elements last
            hud_update(player);

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
        if (!player || !player->_inuse)
        {
            done = 1;
        }
        else if (player->position.y + player->bounds.y > level->height * level->tileDef->height)
        {
            player_kill("Player fell from a high place");
        }
        
        if (keys[SDL_SCANCODE_ESCAPE] || !player || !player->_inuse)done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    slog("---==== END ====---");
    music_queue_free();
    return 0;
}
/*eol@eof*/
