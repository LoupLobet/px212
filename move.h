#include "sokoban.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h> 

int move (Map *m, Pair move,Stack **s);
Stack * pushstack(Stack **s, Pair move, int boxmoved);
int popstack(Stack **s, Stack *pop);
int undomove(Stack **movement, Map *m);
int canwemove(Map *m, Pair move);

