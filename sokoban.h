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
} Map;


typedef struct  {
    Pair move;
    int boxmoved;// Boolean : if a box was moved
    struct Stack *prev;
} Stack;

enum {PLAYER, BOX, EMPTY};
enum {WALL, FLOOR, TARGET};