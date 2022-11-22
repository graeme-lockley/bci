#ifndef MAP_H
#define MAP_H

typedef struct Map_Node
{
    void *key;
    void *value;

    struct Map_Node *left;
    struct Map_Node *right;
} Map_Node;

typedef struct
{
    Map_Node *root;
    int (*compare)(void *a, void *b);
    void (*free_key)(void *key);
    void (*free_value)(void *value);
} Map;

extern Map *map_new(int (*compare)(void *a, void *b), void (*free_key)(void *key), void (*free_value)(void *value));
extern Map *map_string_new(void (*free_value)(void *value));

extern void map_free(Map *map);

extern void *map_get(Map *map, void *key);
extern int map_add(Map *map, void *key, void *value);
extern int map_set(Map *map, void *key, void *value);

extern void free_void(void *key);

#endif
