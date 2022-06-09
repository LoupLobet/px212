#include "sokoban.h"

void	 freemap(Map *);
int	 getmaxlevel(char *);
Map	*loadmap(char *, int);
Stack	*loadsave(char *, int);
Stack	*loadsolve(char *, int);
int	 savemap(Map *, Stack *, char *);
int	 savesolve(Map *, Stack *, char *);

