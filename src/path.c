#include "simple_logger.h"

#include "gfc_list.h"

#include "entity.h"
#include "level.h"
#include "path.h"

typedef struct PNode_S
{
	GFC_Vector2D grid_pos;
	struct PNode_S* parent;
}PathNode;

GFC_List* todo;
GFC_List* done;
Uint8* visited;
PathNode* dest;
GFC_List* found_path;

void path_clear(GFC_List* list)
{
	int i;
	if (!list || !gfc_list_count(list)) return;
	for (int i = 0; i < gfc_list_count(list); i++)
	{
		free(gfc_list_get_nth(list, i));
	}
	gfc_list_clear(list);
}

void path_close()
{
    if (todo)
    {
        path_clear(todo);
        free(todo);
        todo = NULL;
    }
    if (done)
    {
        path_clear(done);
        free(done);
        done = NULL;
    }
    if (visited)
    {
        free(visited);
        visited = NULL;
    }
    if (found_path)
    {
        gfc_list_clear(found_path);
        free(found_path);
        found_path = NULL;
    }
}

void path_init()
{
    if (todo || done || visited || found_path)
    {
        return;
    }

    Level* cur;
    cur = get_current_level();
    if (!cur)
    {
        return;
    }

	todo = gfc_list_new();
	if (!todo) return;

    done = gfc_list_new();
    if (!done)
    {
        free(todo);
        return;
    }

    visited = gfc_allocate_array(sizeof(Uint8), cur->width * cur->height);
    if (!visited)
    {
        free(todo);
        free(done);
        return;
    }

    found_path = gfc_list_new();
    if (!found_path)
    {
        free(todo);
        free(done);
        free(visited);
        return;
    }
}

int node_test(float new_x, float new_y, GFC_Vector2D old)
{
    Level* cur;
    int ind_below, ind, ind_test, i, j, k;
    cur = get_current_level();
    if (!cur) return 0;
    ind_below = level_get_tile_index(cur, new_x, new_y + 1);
    ind = level_get_tile_index(cur, new_x, new_y);
    if (ind < 0 || tile_is_solid(cur->tileMap[ind], 0))
    {
        return 0;
    }
    if (ind_below >= 0 && tile_is_solid(cur->tileMap[ind_below], 1))
    {
        return 1;
    }
    if (new_y < old.y)
    {
        k = 1;
        // jumping logic
        for (i = new_y + 1; i < cur->height; i++)
        {
            ind_test = level_get_tile_index(cur, new_x, i);
            if (ind_test >= 0 && tile_is_solid(cur->tileMap[ind_test], 1))
            {
                k = i - new_y;
                break;
            }
        }

        for (i = new_y; i > new_y - 7 + k; i--)
        {
            ind_test = level_get_tile_index(cur, new_x, i);
            if (ind_test >= 0 && tile_is_solid(cur->tileMap[ind_test], 0))
            {
                return 0;
            }
            for (j = new_x - 4; j < new_x + 4; j++) {
                if (j == 0) continue;
                ind_test = level_get_tile_index(cur, j, i);
                ind_below = level_get_tile_index(cur, j, i + 1);
                if (ind_test >= 0 && !tile_is_solid(cur->tileMap[ind_test], 0) &&
                    ind_below >= 0 && tile_is_solid(cur->tileMap[ind_below], 1))
                {
                    return 1; // solid tile within jump range
                }
            }
        }
    }
    else
    {
        // falling logic
        for (i = new_y + 1; i < cur->height; i++)
        {
            ind_test = level_get_tile_index(cur, new_x, i);
            if (ind_test >= 0 && tile_is_solid(cur->tileMap[ind_test], 1))
            {
                return 1; // hits a solid tile eventually
            }
        }
    }
    return 0;
}

PathNode* create_node(GFC_Vector2D position, PathNode* parent)
{
    PathNode* node;
    node = gfc_allocate_array(sizeof(PathNode), 1);
    if (!node) return NULL;

    node->grid_pos = position;
    node->parent = parent;
    return node;
}

float distance_1d(GFC_Vector2D start, GFC_Vector2D end)
{
    int dx, dy;
    dx = start.x - end.x;
    dy = start.y - end.y;
    return abs(dx) + abs(dy);
}

float distance_2d(GFC_Vector2D start, GFC_Vector2D end)
{
    int dx, dy;
    dx = start.x - end.x;
    dy = start.y - end.y;
    return SDL_sqrt(dx * dx + dy * dy);
}

int tiles_off_ground(GFC_Vector2D position)
{
    int i, ind;
    Level* cur;
    cur = get_current_level();
    if (!cur) return -1;
    for (int i = 0; i < cur->height - position.y - 1; i++)
    {
        ind = level_get_tile_index(cur, position.x, position.y + i + 1);
        if (ind >= 0 && tile_is_solid(cur->tileMap[ind], 1)) return i;
    }
    return -1;
}

void add_node_todo(GFC_Vector2D position, PathNode* parent, GFC_Vector2D end)
{
    PathNode* node, *node2;
    float dist1, dist2;
    int i;
    Level* cur;

    if (!todo) return NULL;
    node = create_node(position, parent);
    if (!node) return NULL;

    cur = get_current_level();
    i = level_get_tile_index(cur, position.x, position.y);
    if (i < 0 || visited[i])
    {
        free(node);
        return;
    }
    visited[i] = 1;

    // use tiles_off_ground to penalize options that prioritize jumping
    dist1 = distance_1d(position, end) + tiles_off_ground(position);
    for (i = 0; i < gfc_list_count(todo); i++)
    {
        node2 = (PathNode*)gfc_list_get_nth(todo, i);
        dist2 = distance_1d(node2->grid_pos, end) + tiles_off_ground(position);
        if (dist1 <= dist2)
        {
            gfc_list_insert(todo, (void*)node, i);
            return;
        }
    }

    gfc_list_append(todo, (void*)node);
}

int path_find_end(GFC_Vector2D end_pos)
{
    PathNode* node;
    GFC_Vector2D newPosition;
    newPosition = gfc_vector2d(0, 0);
    while (gfc_list_get_count(todo))
    {
        node = gfc_list_get_nth(todo, 0);
        if (!node)break; //this shouldn't happen
        gfc_list_append(done, node); //add us to the done pile
        gfc_list_delete_data(todo, node); //remove us from the todo pile
        if (gfc_vector2d_compare(node->grid_pos, end_pos))
        {
            dest = node;
            return 1; //we reached our goal
        }
        //now add my kids to the todo pile
        //this is how you handle it where each node is a tile, and only connects to adjacent nodes that are
        //walkable ie: this needs to be determined by the test function pointer
        gfc_vector2d_set(newPosition, node->grid_pos.x - 1, node->grid_pos.y);
        if (node_test(newPosition.x, newPosition.y, node->grid_pos) == 1)
        {
            add_node_todo(newPosition, node, end_pos);
        }
        else if (gfc_vector2d_compare(newPosition, end_pos))
        {
            //if we add the goal, it doesn't matter if it was walkable or not
            add_node_todo(newPosition, node, end_pos);
            continue;
        }

        gfc_vector2d_set(newPosition, node->grid_pos.x + 1, node->grid_pos.y);
        if (node_test(newPosition.x, newPosition.y, node->grid_pos) == 1)
        {
            add_node_todo(newPosition, node, end_pos);
        }
        else if (gfc_vector2d_compare(newPosition, end_pos))
        {
            add_node_todo(newPosition, node, end_pos);
            continue;
        }

        gfc_vector2d_set(newPosition, node->grid_pos.x, node->grid_pos.y - 1);
        if (node_test(newPosition.x, newPosition.y, node->grid_pos) == 1)
        {
            add_node_todo(newPosition, node, end_pos);
        }
        else if (gfc_vector2d_compare(newPosition, end_pos))
        {
            add_node_todo(newPosition, node, end_pos);
            continue;
        }

        gfc_vector2d_set(newPosition, node->grid_pos.x, node->grid_pos.y + 1);
        if (node_test(newPosition.x, newPosition.y, node->grid_pos) == 1)
        {
            add_node_todo(newPosition, node, end_pos);
        }
        else if (gfc_vector2d_compare(newPosition, end_pos))
        {
            add_node_todo(newPosition, node, end_pos);
            continue;
        }
    }

    //we have nothing left to search and we still haven't found it
    return 0;
}

void trace_route()
{
    PathNode* node;
    if (!dest) return;
    if (!found_path) return;
    node = dest;
    while (node->parent)
    {
        gfc_list_insert(found_path, (void*)node, 0);
        node = node->parent;
    }
}

GFC_Vector2D find_next(GFC_Vector2D start_pos, GFC_Vector2D end_pos, double dmax, int xmax, int ymax, Uint8 reuse)
{
	GFC_Vector2D start_grid_pos, end_grid_pos;
    int dx, dy;
    PathNode* node;
    Level* cur;

	start_grid_pos = gfc_vector2d(roundf(start_pos.x / 32), roundf(start_pos.y / 32));
	end_grid_pos = gfc_vector2d(roundf(end_pos.x / 32), roundf(end_pos.y / 32));
    start_pos.x = start_grid_pos.x * 32;
    start_pos.y = start_grid_pos.y * 32;

    dx = start_grid_pos.x - end_grid_pos.x;
    dy = start_grid_pos.y - end_grid_pos.y;

    if (!dx && !dy)
    {
        return start_pos;
    }
    if (dmax > 0 && distance_1d(start_grid_pos, end_grid_pos) > dmax)
    {
        return start_pos;
    }
    if (xmax > 0 && (dx > xmax || dx < -1 * xmax))
    {
        return start_pos;
    }
    if (ymax > 0 && (dy > ymax || dy < -1 * ymax))
    {
        return start_pos;
    }

    if (!reuse)
    {
        cur = get_current_level();
        path_clear(todo);
        path_clear(done);
        memset(visited, 0, cur->width * cur->height * sizeof(Uint8));
        dest = NULL;
        gfc_list_clear(found_path);

        // checks
        add_node_todo(start_grid_pos, NULL, end_grid_pos);
        if (!path_find_end(end_grid_pos))
        {
            return start_pos;
        }
        trace_route();
    }

    node = gfc_list_get_nth(found_path, 0);
    if (!node)
    {
        return start_pos;
    }
    gfc_list_delete_nth(found_path, 0);
    node->grid_pos.x *= 32;
    node->grid_pos.y *= 32;
    return node->grid_pos;
}