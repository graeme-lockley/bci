#include <string.h>

#include "memory.h"

#include "map.h"

Map *map_new(int (*compare)(void *a, void *b))
{
    Map *map = ALLOCATE(Map, 1);

    map->root = NULL;
    map->compare = compare;

    return map;
}

Map *map_string_new(void)
{
    return map_new((int (*)(void *, void *))strcmp);
}

static void map_free_node(Map_Node *node)
{
    if (node)
    {
        map_free_node(node->left);
        map_free_node(node->right);
        FREE(node->key);
        FREE(node->value);
        FREE(node);
    }
}

void map_free(Map *map)
{
    map_free_node(map->root);

    FREE(map);
}

void *map_get(Map *map, void *key)
{
    Map_Node *current = map->root;
    while (1)
    {
        if (current == NULL)
        {
            return NULL;
        }
        int cmp = map->compare(key, current->key);
        if (cmp == 0)
        {
            return current->value;
        }
        else if (cmp < 0)
        {
            current = current->left;
        }
        else
        {
            current = current->right;
        }
    }
}

static Map_Node *map_node_new(void *key, void *value)
{
    Map_Node *node = ALLOCATE(Map_Node, 1);

    node->key = key;
    node->value = value;

    node->left = NULL;
    node->right = NULL;

    return node;
}

int map_add(Map *map, void *key, void *value)
{
    Map_Node **current = &map->root;
    while (1)
    {
        if (*current == NULL)
        {
            *current = map_node_new(key, value);
            return 1;
        }

        int cmp = map->compare(key, (*current)->key);
        if (cmp == 0)
        {
            return 0;
        }
        else if (cmp < 0)
        {
            current = &(*current)->left;
        }
        else
        {
            current = &(*current)->right;
        }
    }
}

int map_set(Map *map, void *key, void *value)
{
    Map_Node *current = map->root;
    while (1)
    {
        if (current == NULL)
        {
            return 0;
        }
        int cmp = map->compare(key, current->key);
        if (cmp == 0)
        {
            FREE(current->value);
            current->value = value;
            return 1;
        }
        else if (cmp < 0)
        {
            current = current->left;
        }
        else
        {
            current = current->right;
        }
    }
}
