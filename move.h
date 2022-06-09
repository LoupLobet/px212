#include "sokoban.h"

int	 canmove(Map *m, Pair move);
void	 cursormove(Map *m);
void	 execstack(Stack *input,Map *m,Stack **output);
int	 move(Map *m, Pair move,Stack **s);
int	 popstack(Stack **s, Stack *pop);
Stack	*pushstack(Stack **s, Pair move, int boxmoved);
int	 undomove(Stack **movement, Map *m);
