#ifndef SOKOBAN_STRUCT_DEF
#define SOKOBAN_STRUCT_DEF

enum {PLAYER = '@', BOX = '$', EMPTY};
enum {WALL = '#', FLOOR = ' ', TARGET = '.'};

typedef struct  {
    int type;
    int content;
} Space;

typedef struct {
    int x;
    int y;
} Pair;

typedef struct {
    Pair size;
    Pair player;
    Pair cursor;
    Space **grid;
    char *comment;
    int id;
    char *author;
    int strokes;
} Map;

typedef struct Stack {
    Pair move;
    int boxmoved;
    struct Stack *prev;
} Stack;

#endif
