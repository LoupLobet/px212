#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "display.h"
#include "input.h"
#include "loader.h"
#include "move.h"
#include "sokoban.h"
#include "util.h"

static void	usage(void);

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
 * @brief This function verify if each target of the level have a box on it by being launched after every move.
 *
 * @param m Map
 * @return int
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

static Map *
loadlevel(char *file, int level, Stack **s)
{
	Map *m;
	Stack *save;

	if ((m = loadmap(file, level)) == NULL)
		return NULL;
	m->strokes = 0;
	if ((save = loadsave(file, level)) != NULL) {
		displaystr("save detected, load it ? (y/n)");
		if (io() == 'y')
			m->strokes = execstack(save, m, s);
		freestack(&save);
	}
	return m;
}

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
 * @brief This function is the global structure of our program. We manage in it all the functions, how the game works by launching levels, know if we win and do the user interface.
 *
 * @param argc
 * @param argv
 * @return int
 */
int
main(int argc, char *argv[])
{
	char c;
	int level;
	int maxlevel;
	char *file;
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
	if ((m = loadlevel(file, level, &s)) == NULL)
		error("could not load level %d from file: %s", level, file);
//	if ((maxlevel = getmaxlevel(file)) == -1)
//		error("absent or ill formed MAXLEVEL tag in file: %s", file);
maxlevel = 3;
	if (!configureTerminal())
		error("could not configure terminal");
	display(m);


	while ((c = io()) != 27) { /* 27 esc */
		switch (c) {
		case 'U':
			move(m, (Pair){0, -1}, &s);
			break;
		case 'D':
			move(m, (Pair){0, 1}, &s);
			break;
		case 'L':
			move(m, (Pair){-1, 0}, &s);
			break;
		case 'R':
			move(m, (Pair){1, 0}, &s);
			break;
		case 'z':
			undomove(m, &s);
			break;
		case 's':
			if (savemap(m,s,file))
				displaywarning("couldn't save the level %d to file: %s", m->id, file);
			break;
		case 'l':
			level = estrtol(getstr(), 10);
			if ((m = changelevel(m, file, level, &s)) == NULL)
				error("could not load level %d from file: %s", level, file);
			break;
		case 'r':
			if ((m = changelevel(m, file, level, &s)) == NULL)
				error("could not load level %d from file: %s", level, file);
			break;
		case 'p':
			if (level > 1) {
				level--;
				if ((m = changelevel(m, file, level, &s)) == NULL)
					error("could not load level %d from file: %s", level, file);
			} else
				displaystr("no previous level");
			break;
NEXT:	case 'n':
			if (level < maxlevel) {
				level++;
				if ((m = changelevel(m, file, level, &s)) == NULL)
					error("could not load level %d from file: %s", level, file);
			} else
				displaystr("Ok well played ... I admit ...");
			break;
		case 'c':
			cursormove(m);
			continue;
			break;
		default:
			displaywarning("invalid input: %c", c);
		}
		display(m);

		if (iswin(m)) {
			displaystr("Wow ... you won, I didn't expect that from you. Give a try to the next one !");
			if (io() == 'y')
				goto NEXT;
		}
	}
	return 0;
}

static void
usage(void)
{
	fputs("usage: sokoban file [level]\n", stderr);
	exit(1);
}