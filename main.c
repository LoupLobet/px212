#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ia.h"
#include "display.h"
#include "input.h"
#include "loader.h"
#include "move.h"
#include "sokoban.h"
#include "util.h"

static Map	*changelevel(Map *, char *, int, Stack **);
static void	 cursormode(Map *, Stack **);
static char	*getstr(void);
static int	 iswin(Map *);
static Map	*loadlevel(char *, int, Stack **);
static int	 showsolve(Map *m, char *file);
static void	 usage(void);

/**
 * @brief This function is the global structure of our program.
 * We manage in it all the functions, how the game works by launclevels, know if we win and do the user interface.
 * Menu : 
 * 
 * - Press 'ESC' to quit 
 * - Press Arrow Keys to move
 * - Press 'z' to undo
 * - Press 's' to save
 * - Press 'r' to restart
 * - Press 'l' nblevel and enter to choose a new level
 * - Press 'n' to go to next level
 * - Press 'p' to go to previous level
 * - Press 'c' to pass in cursor mode and 'ESC' to exit\
 * - Press 'x' to show a solution
 * 
 * @param argc
 * @param argv
 * @return int
 */
int
main(int argc, char *argv[])
{
	char c;
	int level = 1;
	int maxlevel;
	char *file = NULL;
	Map *m;
	Stack *s;

	if (argc == 3) {
		file = argv[1];
		level = estrtol(argv[2], 10);
	} else if (argc == 2) {
		file = argv[1];
		level = 1;
	} else
		usage();

	s = NULL;
	if (!configterm())
		error("could not configure terminal");
	if ((m = loadlevel(file, level, &s)) == NULL)
		error("could not load level %d from file: %s", level, file);
	if ((maxlevel = getmaxlevel(file)) == -1)
		error("absent or ill formed MAXLEVEL tag in file: %s", file);

	display(m);

	while ((c = io()) != 27) { /* 27 esc */
		switch (c) {
		case 'U':
			move(m, (Pair){0, -1}, &s);
			display(m);
			break;
		case 'D':
			move(m, (Pair){0, 1}, &s);
			display(m);
			break;
		case 'L':
			move(m, (Pair){-1, 0}, &s);
			display(m);
			break;
		case 'R':
			move(m, (Pair){1, 0}, &s);
			display(m);
			break;
		case 'z':
			undomove(m, &s);
			display(m);
			break;
		case 's':
			if (savemap(m,s,file))
				error("couldn't save the level %d to file: %s", m->id, file);
			displaystr("level saved !");
			break;
		case 'l':
			level = estrtol(getstr(), 10);
			if ((m = changelevel(m, file, level, &s)) == NULL)
				error("could not load level %d from file: %s", level, file);
			display(m);
			break;
RESET:	case 'r':
			if ((m = changelevel(m, file, level, &s)) == NULL)
				error("could not load level %d from file: %s", level, file);
			display(m);
			break;
		case 'p':
			if (level > 1) {
				level--;
				if ((m = changelevel(m, file, level, &s)) == NULL)
					error("could not load level %d from file: %s", level, file);
				display(m);
			} else
				displaystr("no previous level");
			break;
NEXT:	case 'n':
			if (level < maxlevel) {
				level++;
				if ((m = changelevel(m, file, level, &s)) == NULL)
					error("could not load level %d from file: %s", level, file);
				display(m);
			} else
				displaystr("no next level");
			break;
		case 'x':
			if (showsolve(m, file)) {
				display(m);
				displaywarning("No solve available, you have not won this level yet");
			} else {
				if ((m = changelevel(m, file, level, &s)) == NULL)
					error("could not load level %d from file: %s", level, file);
				display(m);
			}
			break;
		case 'c':
			cursormode(m, &s);
			display(m);
			break;
		default:
			displaywarning("invalid input: %c", c);
		}

		if (iswin(m)) {
			displaystr("Wow !!! What a chad ! You won ! Give a try to the next one ! (y/n)");
			savesolve(m, s, file);
			if (io() == 'y')
				goto NEXT;
			else
				goto RESET;

		}
	}
	display(m);
	resetterm();
	freemap(m);
	freestack(&s);
	return 0;
}

/**
 * @brief This function allow the user to specify which level he want to
 * play and launch it.
 * @param m Map
 * @param s Stack of movement
 * @return int return number of strokes
 */
static Map *
changelevel(Map *m, char *file, int level, Stack **s)
{
	freemap(m);
	freestack(s);
	m = loadlevel(file, level, s);
	/* loadlevel return NULL -> changelevel returns NULL */
	return m;
}

/**
 * @brief This function allows the user to move the cursor placed
 * first at the player position when he pass to cursor mode.This
 * function is the beginning for advanced functions when we want
 * to specify a destination to go.
 * @param m Map
 * @return void
 */
static void
cursormode(Map *m, Stack **s)
{
	Stack *path;
	Stack pop;
	char c;

	setcursor(m, m->player);
	while ((c = io()) != 27) {
		switch (c) {
		case 'U':
			movecursor(m, (Pair){ 0, -1 });
			break;
		case 'D':
			movecursor(m, (Pair){ 0, 1 });
			break;
		case 'L':
			movecursor(m, (Pair){ -1, 0 });
			break;
		case 'R':
			movecursor(m, (Pair){ 1, 0 });
			break;
		case 10: /* enter */
			display(m);
			path = playerwayto(m, m->cursor);
			while (!popstack(&path, &pop)) {
				usleep(250000);
				move(m, pop.move, s);
				display(m);
			}
			setcursor(m, m->player);
			break;
		}
	}
	setcursor(m, (Pair){ -1, -1 });
}

/**
 * @brief This function allows the user to enter a string by writing
 * it and press enter in the new configuration terminal.
 * @return char* Return the string entered.
 */
static char *
getstr(void)
{
	int i, size;
	char c;
	char *s;

	size = 4;
	s = emalloc(size);
	for (i = 0; (c = io()) != 10; i++) {
		if (i > size - 2)
			s = realloc(s, size * 2);
		s[i] = c;
	}
	s[i] = '\0';
	return s;
}

/**
 * @brief This function verify if each target of the level have a box
 * on it and so on if we win, by being launched after every move.
 * @param m Map
 * @return int Return 1 if we win, 0 otherwise.
 */
static int
iswin(Map *m)
{
	int i, j;
	for (j = 0; j < m->size.y; j++) {
		for (i = 0; i < m->size.x; i++) {
			if (m->grid[i][j].type == TARGET && m->grid[i][j].content != BOX)
				return 0;
		}
	}
	return 1;
}

/**
 * @brief This function load a map and verify if a save is detected in
 * the file "file". If yes, we can load it by executing the movement
 * saved in the file with execstack.
 * 
 * @param file
 * @param level
 * @param s Stack of movement
 * @return Map* Return the map loaded.
 */
static Map *
loadlevel(char *file, int level, Stack **s)
{
	Map *m;
	Stack *save;

	if ((m = loadmap(file, level)) == NULL)
		return NULL;
	m->strokes = 0;
	display(m);
	if ((save = loadsave(file, level)) != NULL) {
		displaystr("save detected, load it ? (y/n)");
		if (io() == 'y')
			m->strokes = execstack(save, m, s);
		/* save is freed by execstack calling popstack */
	}
	return m;
}

/**
 * @brief This function show the solution of the level if it exists step by step.
 * 
 * @param m Map
 * @param file File that we want to load
 * @return int Return 0 if we can show the solution, 0 otherwise
 */

static int
showsolve(Map *m, char *file)
{
	Stack *solve;
	Stack *ds;
	Stack pop;

	solve = loadsolve(file, m->id);
	if (solve != NULL) {
		while (!popstack(&solve, &pop)) {
			usleep(250000);
			move(m, pop.move, &ds);
			display(m);
		}
		return 0;
	}
	return 1;
}

/**
 * @brief This function give to the user the good usage to load the program.
 * 
 */

static void
usage(void)
{
	fputs("usage: sokoban file [level]\n", stderr);
	exit(1);
}
