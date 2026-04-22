#ifndef __EDITOR_H__
#define __EDITOR_H__

/**
* @brief checks whether the editor window is open
* @return 0 if not open, 1 if open
*/
Uint8 is_editor_open();

/**
* @brief creates an editor window
*/
Window* window_editor();

#endif