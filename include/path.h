#ifndef __PATH_H__
#define __PATH_H__

#endif

/**
* @brief closes the pathfinding system
*/
void path_close();

/**
* @brief initializes the pathfinding system
*/
void path_init();

/**
* @brief finds the Manhattan distance between two points
* @param start the position to start from
* @param end the intended destination
* @return the distance between the two points
*/
float distance_1d(GFC_Vector2D start, GFC_Vector2D end);

/**
* @brief finds the Pythagorean distance between two points
* @param start the position to start from
* @param end the intended destination
* @return the distance between the two points
*/
float distance_2d(GFC_Vector2D start, GFC_Vector2D end);

/**
* @brief finds the next position to head towards
* @param start_pos the position to start from
* @param end_pos the intended destination
* @param dmax max 2D distance to check, in tiles (ignored if <= 0)
* @param xmax max horizontal distance to check, in tiles (ignored if <= 0)
* @param ymax max vertical distance to check, in tiles (ignored if <= 0)
* @param reuse whether or not to reuse results from last time (saves time, but can be inaccurate)
* @return start_pos if path not found, next position to head towards if path found
*/
GFC_Vector2D find_next(GFC_Vector2D start_pos, GFC_Vector2D end_pos, double dmax, int xmax, int ymax, Uint8 reuse);