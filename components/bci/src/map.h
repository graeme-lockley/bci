#ifndef MAP_H
#define MAP_H

typedef struct Map_Node {
    void *key;
    void *value;

    struct Map_Node *left;
    struct Map_Node *right;
} Map_Node;

typedef struct {
    Map_Node *root;
    int (*compare)(void *a, void *b);
} Map;

Map *map_new(int (*compare)(void *a, void *b));
Map *map_string_new(void);

void map_free(Map *map);

void *map_get(Map *map, void *key);
int map_add(Map *map, void *key, void *value);
int map_set(Map *map, void *key, void *value);

#endif
