#ifndef __SKILL_TREE_H__
#define __SKILL_TREE_H__

/**
* @brief checks whether the skill tree window is open
* @return 0 if not open, 1 if open
*/
Uint8 is_skill_tree_open();

/**
* @brief creates a skill tree window
*/
Window* window_skill_tree();

#endif