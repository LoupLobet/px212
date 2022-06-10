#ifndef SOKOBAN_STRUCT_DEF
#define SOKOBAN_STRUCT_DEF

enum {PLAYER = '@', BOX = '$', EMPTY};
enum {WALL = '#', FLOOR = ' ', TARGET = '.'};

/**
 * @brief This structure is used to specify what is ( type : WALL,FLOOR, TARGET and content : PLAYER,BOX,EMTPY)
 *  in every space of the grid.
 * 
 */
typedef struct  {
    int type;
    int content;
} Space;

/**
 * @brief This structure is used to specify a position in x and y .
 * 
 */

typedef struct {
    int x;
    int y;
} Pair;

/**
 * @brief This structure is the principal of our programm. We manage in it the map and so on 
 * the grid of the map and all of his parameters.
 * 
 */
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

/**
 * @brief This structure is used to save movement.
 * 
 */
typedef struct Stack {
    Pair move;
    int boxmoved;
    struct Stack *prev;
} Stack;

#endif
