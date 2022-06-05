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



Map* load_map(Map *m, int level, char *file){
	m = loadmap(file, level);
	if (m == NULL) {
		error("could not load map");
	}
	return m;

}

void display_temp(Map *m, int coup){
	int i, j;
	printf("\e[1;1H\e[2J");
	printf("\nYou made %d strokes\n",coup);
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
	printf("\n - Press 'ESC' to quit \n - Press Arrow Keys to move\n - Press 'z' to undo\n - Press 's' to save\n - Press 'r' to restart\n");
	
}

Map* initialisation(Map *m){
	printf("\n=======Game of Sokoban===========\n");
	printf("\nPress 'a' to load an old save\n");
	printf("\nPress 'b' to load a new game\n\n\n");
	int choose = io();
	if (choose == 'a'){
		char *file = "levels.save";
		m = load_map(m,0,file);
	}
	else if (choose == 'b'){
		char *file = "levels.lvl";
		m = load_map(m,1,file);
		display_temp(m,0);
	}
	else{
		printf("\nWrong choice\n");
		exit(EXIT_FAILURE);
	}
	return m;

}


int
main(int argc, char *argv[])
{

	Map *m;
	Stack *s;
	int choix ;
	int coup = 0;
	int level = 1;

	if (!configureTerminal())
	{
		error("Impossible de configurer le terminal");
		return 0;
	}
	
	

	m = initialisation(m);

	
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
				return 0;
			}
			else if (choix == 'r'){
				coup = 0;
				char *file = "levels.lvl";
				m = load_map(m, level,file);
			}
			display_temp(m,coup);
		
		}
		else{// TODO: make display function and save solution
			display_temp(m,coup);
			
			
			level ++;
			int tmp = coup +1 ;
			coup = 0;
			char *file = "levels.lvl";
			m = load_map(m,level,file);
			display_temp(m,coup);
			printf("\nYou win level %d in %d strokes , you are level %d\n",level,tmp, level--);

		}
	}
		

	return 0;
    
}
