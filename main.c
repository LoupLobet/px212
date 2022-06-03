#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "sokoban.h"
#include "loader.h"
#include "move.h"

int
main(int argc, char *argv[])
{
	int i, j;
	Map *m;
	Stack *s;
	Stack pop;

	m = loadmap("levels.lvl", 69);
	if (m == NULL) {
		error("could not load map");
	}

	for (j = 0; j < m->size.y; j++) {
		for (i = 0; i < m->size.x; i++) {
			if (m->grid[i][j].content != EMPTY)
				putchar(m->grid[i][j].content);
			else
				putchar(m->grid[i][j].type);
		}
		putchar('\n');
	}
	freemap(m);

	s = NULL;
	pushstack(&s, (Pair){ 1, 0 }, 0);
	pushstack(&s, (Pair){ 1, 0 }, 0);
	pushstack(&s, (Pair){ 0, -1}, 0);

	while (!popstack(&s, &pop))
		printf("{ %d, %d }, %d\n", pop.move.x, pop.move.y, pop.boxmoved);

	return 0;
}
