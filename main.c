#include "sokoban.h"


Map* initialisation(Map *m,char *file, int level){
	displaystr("\n=======Game of Sokoban===========\n");

	m = loadingmap(m,level,file);
	displaymap(m,0);
	
	return m;

}

Map* loadingmap(Map *m, int level, char *file){
	m = loadmap(file, level);
	if (m == NULL) {
		displaywarning("Could not load map");
		error("could not load map");
	}
	return m;

}

void displaymap(Map *m, int stroke){
	display(m, stroke);
	displaystr("\n - Press 'ESC' to quit \n - Press Arrow Keys to move\n - Press 'z' to undo\n - Press 's' to save\n - Press 'r' to restart\n - Press l nblevel and enter to choose a new level\n - Press 'n' to go to next level\n - Press 'p' to go to previous level\n - Press 'c' to pass in cursor mode and 'ESC' to exit\n");
	
}


int nextlevel(Map *m, int level){
	int stroke = 0;
	char *file = "levels.lvl";
	freemap(m);
	level++;
	m = loadingmap(m, level,file);
	return stroke;

}

int prevlevel(Map *m, int level){
	if (level == 1){
		printf("You are on the first level !\n");
		return 0;
	}
	
	int stroke = 0;
	char *file = "levels.lvl";
	freemap(m);
	level--;
	m = loadingmap(m, level,file);
	return stroke;
}

int changelevel(Map *m){
	int stroke = 0;
	int size = 0;
	int n = 0;
	int niveau[10] = {-1};
	char temp;
	while ((temp = io()) != 10)
		{
			niveau[n]=temp - '0';
			n++;
		}
				
	size = n;
	int i =  0;
	int level = 0;
	for (i = 0; i < size; i++)
    	level = 10 * level + niveau[i];
	
	char *file = "levels.lvl";
	freemap(m);
	m = loadingmap(m, level,file);
	return stroke;

}

int restart(Map *m,int level){
	int stroke = 0;
	char *file = "levels.lvl";
	freemap(m);
	m = loadingmap(m, level,file);
	return stroke;
}

int saveplan(Map *m, Stack *s){
	//savemap(m, s, "levels.save");
	
}

int undomovement(Map *m,Stack *s, int stroke){
		if (stroke == 0)
		{
			printf("You can't undo anymore !\n");
			return 0;
		}
		stroke --;
		
		
		undomove(&s,m);
		return stroke;
}

void cursormove(Map *m){
	char mov;
	setcursor(m,(Pair){m->player.x,m->player.y});
	displaycursor();
	while ((mov = io()) != 27){
	
			switch (mov) {
				case 'U': movecursor(m, (Pair){0,-1}); break;
				case 'D': movecursor(m, (Pair){0,1}); break;
				case 'L': movecursor(m, (Pair){-1,0}); break;
				case 'R': movecursor(m, (Pair){1,0}); break;
				default:displaywarning("Invalid move"); break;
			}
			displaycursor();
	}
}


int verifiegagne(Map *m){
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



int main(int argc, char *argv[])
{

	Map *m;
	Stack *s;
	Stack *p;
	int choix ;
	int stroke = 0;
	int level = 1;
	int tmp;
	char temp;
	char *file;
	int i = 0;
	char mystr[30];
	char soluce[80];
	char mov;




	if (!configureTerminal())
	{
		displaywarning("Could not configure terminal");
		error("Configuration of the terminal is impossible");
		return 0;
	}
	
	
	if (argc == 3)
	{
		
		m = initialisation(m,argv[1],atoi(argv[2]));
	}
	else if (argc == 2)
	{
		m = initialisation(m,argv[1],1);
	}
	else if (argc == 1)
	{
		m = initialisation(m,"levels.lvl",1);
	}
	else{
		displaywarning("Wrong number of arguments");
		error("Problem of argument : Arguments must be ./prog file NbLevel");
		return EXIT_FAILURE;
	}

	
    while ((mov = io()) != 27){
	
			switch (mov) {
				case 'U': move (m,(Pair){0,-1},&s); stroke++; break;
				case 'D': move (m,(Pair){0,1},&s);  stroke++; break;
				case 'L': move (m,(Pair){-1,0},&s); stroke++;break;
				case 'R': move (m,(Pair){1,0},&s);  stroke++;break;
				case 'z': stroke = undomovement(m,s,stroke);break;
				case 'r': stroke = restart(m,level);break;
				case 's': saveplan(m,s); return 0;
				case 'l': stroke = changelevel(m);break;
				case 'p': prevlevel(m,level);level--; break;
				case 'n': stroke = nextlevel(m,level);level++; break;
				case 'c': cursormove(m);continue;break;
				default: displaywarning("Wrong input"); error ("Vous n'avez pas entrez la bonne touche") ; return 0; break;
			
			
			

//			for (i = 0; i < ncommands; i++) {
//				if (commands[i] = choix)
//					(*commands[i].func)(m, s, Args);
//			}
		
		}
	
		
		
		
		
			displaymap(m,stroke);
			
			if (!verifiegagne(m))
			{
				continue;
			}
			else
			
			tmp = stroke +1 ;
			stroke = 0;
			file = "levels.lvl";
			sprintf(mystr, "%d", level);  // convert level to string
			strcpy(soluce, "solucelevel");
			strcat(soluce, mystr);
			strcat(soluce, ".save");
			//savemap(m, s, soluce);
			level ++;
			freemap(m);
			m = loadingmap(m,level,file);
			displaymap(m,stroke);
			printf("\nYou win level %d in %d strokes , you are level %d\n",level,tmp, level--);


		}
		return 0;
}