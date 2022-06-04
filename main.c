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

	if (savemap(m, s, "saves.save"))
		error("could not save map %d to: %d", m->id, "saves.save");
	return 0;
}
