#ifndef __CUTSCENE_H__
#define __CUTSCENE_H__

/**
* @brief Checks if there is currently a cutscene
* @return 1 if yes, 0 if no
*/
Uint8 is_cutscene();

/**
* @brief Frees the current cutscene
*/
void cutscene_free();

/**
* @brief Updates the cutscene (called every frame)
*/
void cutscene_update();

/**
* @brief Draws the current portrait (called every frame)
*/
void cutscene_draw_portrait();

/**
* @brief Loads a cutscene
* @param cutscene The file path of the cutscene to load
*/
void cutscene_load(const char* cutscene);

#endif