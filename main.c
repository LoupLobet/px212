<<<<<<< HEAD
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

//	s = NULL;
//	pushstack(&s, (Pair){ 1, 0 }, 0);
//	pushstack(&s, (Pair){ 1, 0 }, 0);
//	pushstack(&s, (Pair){ 0, -1}, 0);

//	if (savemap(m, s, "saves.save"))
//		error("could not save map %d to: %d", m->id, "saves.save");
	return 0;
}
=======
#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "sokoban.h"



int verifie_gagne(Map *m){
	int i,j;
	for (j = 0; j < m->size.y; j++) {
		for (i = 0; i < m->size.x; i++) {
			if (m->grid[i][j].type == TARGET && m->grid[i][j].content != BOX){
				return 0;
			}
		}
	}
	return 1;
}

int
main(int argc, char *argv[])
{

	    int i, j;
	int k = 0;
	Map *m;
	Stack *s;
	int choix ;
	int coup = 0;
	int level = 1;
	

	m = loadmap("levels.lvl", level);
	if (m == NULL) {
		error("could not load map");
	}
	printf("\nVous avez fait %d coups\n",coup);
	for (j = 0; j < m->size.y; j++) {
		for (i = 0; i < m->size.x; i++) {
			if (m->grid[i][j].content != EMPTY)
				putchar(m->grid[i][j].content);
			else{
				putchar(m->grid[i][j].type);
			}
		}
		putchar('\n');
	}
	printf("\n- Press 'ESC' to quit \n - Press Arrow Keys to move\n - Press 'z' to undo\n - Press 's' to save\n - Press 'r' to restart\n");

	
    while ((choix = which_move(m,&s)) != 27){
		if (!verifie_gagne(m)){
		
			coup ++;
			if (choix == 'z') {
				coup -=2;
				undomove(&s,m);
			}
			else if (choix == 's')
			{
				savemap(m, s, "levels.save");
			}
			else if (choix == 'r'){
				coup = 0;
				m = loadmap("levels.lvl", level);
				if (m == NULL) {
					error("could not load map");
				}
			}
			printf("\nVous avez fait %d coups\n",coup);
			for (j = 0; j < m->size.y; j++) {
					for (i = 0; i < m->size.x; i++) {
						if (m->grid[i][j].content != EMPTY)
							putchar(m->grid[i][j].content);
						else
							putchar(m->grid[i][j].type);
					}
				putchar('\n');
				}
				printf("\n- Press 'ESC' to quit \n - Press Arrow Keys to move\n - Press 'z' to undo\n - Press 's' to save\n - Press 'r' to restart\n");
		
		}
		else{ // TODO: make display function and save solution
			for (j = 0; j < m->size.y; j++) {
					for (i = 0; i < m->size.x; i++) {
						if (m->grid[i][j].content != EMPTY)
							putchar(m->grid[i][j].content);
						else
							putchar(m->grid[i][j].type);
					}
				putchar('\n');
				}
				printf("\n- Press 'ESC' to quit \n - Press Arrow Keys to move\n - Press 'z' to undo\n - Press 's' to save\n - Press 'r' to restart\n");
			printf("\nVous avez gagne le niveau %d en %d coups , passons au suivant\n",level,coup);
			
			level ++;
			coup = 0;
			m = loadmap("levels.lvl", level);
			if (m == NULL) {
				error("could not load map");
			}
			for (j = 0; j < m->size.y; j++) {
				for (i = 0; i < m->size.x; i++) {
					if (m->grid[i][j].content != EMPTY)
						putchar(m->grid[i][j].content);
					else{
						putchar(m->grid[i][j].type);
					}
				}
				putchar('\n');
			}
			printf("\n- Press 'ESC' to quit \n - Press Arrow Keys to move\n - Press 'z' to undo\n - Press 's' to save\n - Press 'r' to restart\n");

		}
	}
		
		

    
	freemap(m);

	s = NULL;
	pushstack(&s, (Pair){ 1, 0 }, 0);
	pushstack(&s, (Pair){ 1, 0 }, 0);
	pushstack(&s, (Pair){ 0, -1}, 0);

	
	return 0;
}
>>>>>>> 3e4ae1c241ebaa2d789fb296a84245a0e030bdb3
