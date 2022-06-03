typedef struct Map {
    Pair size;
    Pair player;
    Pair cursor;
    Space **grid;
    char *comment;
    int id;
    char *author;
};

typedef struct Space {
    int type;
    int content;
};

typedef struct Pair {
    int x;
    int y;
};

typedef struct Stack {
    Pair move;
    int boxmoved;
    struct Stack *prev;
};

enum {PLAYER, BOX, EMPTY};
enum {WALL, FLOOR, TARGET};