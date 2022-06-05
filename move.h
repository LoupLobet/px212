#include "sokoban.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>


//int move (Map *m, int largeur, Stack **movement,int longueur);
//int canwemove(Map *m, int x , int y);
Stack *pushstack(Stack **s, Pair move, int boxmoved);
int	 popstack(Stack **, Stack *);
int	 canmove(Map *m, Pair move);
int	 domove(Map *m, Pair move, Stack **s);
//int undomove(Stack **movement, Map *m);
//int which_move(Map *m,Stack **movement);
