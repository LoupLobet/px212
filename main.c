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

	m = loadmap("levels.lvl", 6);
	if (m == NULL) {
		error("could not load map");
	}

	s = NULL;

	for (j = 0; j < m->size.y; j++) {
		for (i = 0; i < m->size.x; i++) {
			if (m->grid[i][j].content != EMPTY)
				putchar(m->grid[i][j].content);
			else
				putchar(m->grid[i][j].type);
		}
		putchar('\n');
	}

	printf("%d\n", domove(m, (Pair) { 0, 1 }, &s));

	for (j = 0; j < m->size.y; j++) {
		for (i = 0; i < m->size.x; i++) {
			if (m->grid[i][j].content != EMPTY)
				putchar(m->grid[i][j].content);
			else
				putchar(m->grid[i][j].type);
		}
		putchar('\n');
	}

	Stack pop;
	while (!popstack(&s, &pop))
		printf("{ %d, %d }, %d\n", pop.move.x, pop.move.y, pop.boxmoved);

//	if (savemap(m, s, "saves.save"))
//		error("could not save map %d to: %d", m->id, "saves.save");
	return 0;
}

