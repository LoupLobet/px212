#include "sokoban.h"


/**
 * @brief This functions count strokes in a stack of movement
 * 
 * @param s  Stack of movement
 * @return int 
 */
int countstrokes(Stack **s){
	Stack *p;
	int i = 0;
	if (NULL == s)
		return i;
	else{
		
		while (NULL != *s)
		{
			i++;
			p = *s;
			*s = p->prev;
			
		}
	}
	return i;
}


/**
 * @brief This function initialize the map when we launch the program
 * 
 * @param m Map
 * @param file File which contain the levels
 * @param level number of level
 * @param s Stack of movement
 * @return Map* return a pointer of the new map
 */
Map *initialisation(Map *m, char *file, int level, Stack **s)
{
	int stroke;
	m = loadingmap(m, level, file,s);
	listeAffiche(*s);
	stroke = countstrokes(s);
	display(m, stroke);

	return m;
}


/**
 * @brief This function launch a map and the level we want from the file that we pass in parameter.The function verify that the map is well loaded.
 * 
 * @param m Map
 * @param level number of level
 * @param file File which contain the levels
 * @param s Stack of movement
 * @return Map* return a pointer of the new map
 */


Map *loadingmap(Map *m, int level, char *file, Stack **s)
{
	Stack *p;
	char choose;
	m = loadmap(file, level);
	if (m == NULL)
	{
		displaywarning("Could not load map");
		error("could not load map");

	}
	
	if ((p=loadsave(level,file)) != NULL)
	{
		displaystr("\nA save is detected, do you want to load it (y\\n) ? \n");
		if ((choose = io()) == 'y')
		{

			 loadfromstack(p,m,s,0);

		}
		
	}
	return m;
	
	
}

/**
 * @brief This function launch the next level when we press 'n' in the game.
 * 
 * @param m Map
 * @param level number of level
 * @param s Stack of movement
 * @return int return number of strokes
 */

int nextlevel(Map *m, int level, Stack **s)
{
	int stroke = 0;
	char *file = "levels.lvl";
	freemap(m);
	level++;
	m = loadingmap(m, level, file,s);
	stroke = countstrokes(s);

	return stroke;
}

/**
 * @brief This function launch the precedent level when we press 'p' in the game
 * 
 * @param m Map
 * @param level number of level
 * @param s Stack of movement
 * @return int return number of strokes
 */

int prevlevel(Map *m, int level,Stack **s)
{
	char *file = "levels.lvl";
	int stroke = 0;
	if (level == 1)
	{
		printf("You are on the first level !\n");
		return 0;
	}

	
	
	freemap(m);
	level--;
	m = loadingmap(m, level, file,s);
	stroke = countstrokes(s);
	return stroke;
}


/**
 * @brief This function allow the user when he press 'l' + numberLevel + enter in game to specify which level he want to play and launch it.
 * 
 * @param m Map
 * @param s Stack of movement
 * @return int return number of strokes
 */

int changelevel(Map *m, Stack **s)
{
	int stroke = 0;
	int size = 0;
	int n = 0;
	int niveau[10] = {-1};
	char temp;
	char *file = "levels.lvl";
	while ((temp = io()) != 10)
	{
		niveau[n] = temp - '0';
		n++;
	}

	size = n;
	int i = 0;
	int level = 0;
	for (i = 0; i < size; i++)
		level = 10 * level + niveau[i];

	
	freemap(m);
	m = loadingmap(m, level, file,s);
	return level;
}

/**
 * @brief This function allow the user to restart the level he play from the beginning.
 * 
 * @param m Map
 * @param level Number of level
 * @param s Stack of movement
 * @return int return number of strokes
 */

int restart(Map *m, int level, Stack **s)
{
	int stroke = 0;
	char *file = "levels.lvl";
	freemap(m);
	m = loadingmap(m, level, file,s);
	return stroke;
}


/**
 * @brief This function verify that we can undo a movement and launch the function for, if we can.
 * 
 * @param m Map
 * @param s Stack of movement
 * @param stroke Number of strokes
 * @return int return number of strokes
 */

int undomovement(Map *m, Stack **s, int stroke)
{
	if (stroke == 0)
	{
		displaystr("You can't undo anymore !\n");
		
		return 0;
	}
	stroke--;

	undomove(s, m);
	return stroke;
}


/**
 * @brief This function verify if each target of the level have a box on it by being launched after every move.
 * 
 * @param m Map
 * @return int 
 */


int verifiegagne(Map *m)
{
	int i, j;
	for (j = 0; j < m->size.y; j++)
	{
		for (i = 0; i < m->size.x; i++)
		{
			if (m->grid[i][j].type == TARGET && m->grid[i][j].content != BOX)
			{
				return 0;
			}
		}
	}
	return 1;
}

/**
 * @brief This function is the global structure of our program. We manage in it all the functions, how the game works by launching levels, know if we win and do the user interface.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */

int main(int argc, char *argv[])
{

	Map *m;
	Stack *s;
	Stack *p;
	int choix;
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

		m = initialisation(m, argv[1], atoi(argv[2]),&s);
	}
	else if (argc == 2)
	{
		m = initialisation(m, argv[1], 1,&s);
	}
	else if (argc == 1)
	{
		m = initialisation(m, "levels.lvl", 1,&s);
	}
	else
	{
		displaywarning("Wrong number of arguments");
		error("Problem of argument : Arguments must be ./prog file NbLevel");
		return EXIT_FAILURE;
	}

	while ((mov = io()) != 27)
	{

		switch (mov)
		{
		case 'U':
			if (move(m, (Pair){0, -1}, &s))
				stroke++;
			break;
		case 'D':
			if (move(m, (Pair){0, 1}, &s))
				stroke++;
			break;
		case 'L':
			if (move(m, (Pair){-1, 0}, &s))
				stroke++;
			break;
		case 'R':
			if (move(m, (Pair){1, 0}, &s))
				stroke++;
			break;
		case 'z':
			stroke = undomovement(m, &s, stroke);
			break;
		case 'r':
			stroke = restart(m, level,&s);
			break;
		case 's':
			file = "levels.lvl";
			savemap(m,s,file);
			return 0;
		case 'l':
			level = changelevel(m,&s);
			stroke = countstrokes(&s);
			break;
		case 'p':
			prevlevel(m, level,&s);
			level--;
			break;
		case 'n':
			stroke = nextlevel(m, level,&s);
			level++;
			break;
		case 'c':
			cursormove(m);
			continue;
			break;
		default:
			displaywarning("Wrong input");
			error("Vous n'avez pas entrez la bonne touche");
			return 0;
			break;

			//			for (i = 0; i < ncommands; i++) {
			//				if (commands[i] = choix)
			//					(*commands[i].func)(m, s, Args);
			//			}
		}

		display(m, stroke);

		if (!verifiegagne(m))
		{
			continue;
		}
		else

		tmp = stroke + 1;
		stroke = 0;
		file = "levels.lvl";
		//sprintf(mystr, "%d", level); // convert level to string
		
		// savemap(m, s, soluce);
		level++;
		freemap(m);
		m = loadingmap(m, level, file,&s);
		stroke = countstrokes(&s);
		display(m, stroke);
		printf("\nYou win level %d in %d strokes , you are level %d\n", level, tmp, level--);
	}
	return 0;
}

