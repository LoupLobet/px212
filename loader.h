#include "sokoban.h"

void	 freemap(Map *);
int	 getmaxlevel(char *);
Map	*loadmap(char *, int);
int	 savemap(Map *, Stack *, char *);
Stack	*loadsave(char *, int);
