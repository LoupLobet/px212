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
	int k = 0;
	Map *m;
	Stack *s;
	int choix ;
	int coup = 0;
	int nbre_target = 0;
	

	m = loadmap("levels.lvl", 1);
	if (m == NULL) {
		error("could not load map");
	}

	for (j = 0; j < m->size.y; j++) {
		for (i = 0; i < m->size.x; i++) {
			if (m->grid[i][j].content != EMPTY)
				putchar(m->grid[i][j].content);
			else{
				if (m->grid[i][j].type == TARGET){
					nbre_target++;
				}
				
				putchar(m->grid[i][j].type);
			}
		}
		putchar('\n');
	}
	
    while ((choix = which_move(m,&s)) != 27) // 27= ESC Key
    {
        coup ++;
		if (choix == 'z') {
			undomove(&s,m);
		}
		else if (choix == 's')
		{
			if (savemap(m, s, "saves.save"))
				error("could not save map %d to: %d", m->id, "saves.save");
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
     		printf("\n- Press 'ESC' to quit \n - Press Arrow Keys to move\n - Press 'z' to undo\n");
	}

    
	freemap(m);

	s = NULL;
	pushstack(&s, (Pair){ 1, 0 }, 0);
	pushstack(&s, (Pair){ 1, 0 }, 0);
	pushstack(&s, (Pair){ 0, -1}, 0);

	
	return 0;
}
