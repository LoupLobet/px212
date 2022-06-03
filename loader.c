#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "util.h"
#include "sokoban.h"

#define NO_PLAYER { -1, -1 };

enum { AUTHOR, COMMENT, LEVEL, PASS };

union value {
	int i;
	char *s;
};

struct lvl {
	unsigned int line;
	struct lvl *next;
	struct lvl *prev;
};

struct comment {
	int op;
	union value val;
};

static struct comment	*parsecomment(FILE *);
static Pair	 getmapsize(FILE *fp);
static int	 gotolevel(FILE *, int);

struct lvl *table;

int
main(int argc, char *argv[])
{
	int i, j;
	Map *m;

	m = loadmap("levels.lvl", 1);

	for (j = 0; j < m->size.y; j++) {
		for (i = 0; i < m->size.x; i++) {
			if (m->grid[i][j].content != EMPTY)
				putchar(m->grid[i][j].content);
			else
				putchar(m->grid[i][j].type);
		}
		putchar('\n');
	}
}

Map *
loadmap(char *file, int n)
{
	int i;
	int c;
	int x, y;
	struct comment *cmt;
	FILE *fp;
	Map *m;

	if ((fp = fopen(file, "r")) == NULL) {
		warning("could not open file: %s", file);
		return NULL;
	}
	if (gotolevel(fp, n)) {
		warning("no such level: %s", file);
		return NULL;
	}
	m = emalloc(sizeof(Map));
	m->size = getmapsize(fp);
	m->id = n;
	m->player = (Pair)NO_PLAYER;
	m->grid = emalloc(sizeof(Space *) * m->size.x);
	for (i = 0; i < m->size.x; i++)
		m->grid[i] = emalloc(sizeof(Space) * m->size.y);
	/* fill the grid */
	x = 0;
	y = 0;
	while ((c = fgetc(fp)) != EOF) {
		if (c == '\n') {
			x = 0;
			y++;
			continue;
		}
		if (c == ';') {
			if (x == 0 && y == 0) {
				/* additional comment before map grid */
				cmt = parsecomment(fp);
				switch (cmt->op) {
				case AUTHOR:
					m->author = estrdup(cmt->val.s);
					break;
				case COMMENT:
					m->comment = estrdup(cmt->val.s);
					break;
				case LEVEL:
					/* empty map, error */
					warning("empty map: %d", n);
					return NULL;
				case PASS:
					break;
				}
				free(cmt);
			} else {
				/* map grid not empty, new comment signals the end of the map */
				break;
			}
		} else {
			switch (c) {
			case FLOOR:
				m->grid[x][y].type = FLOOR;
				m->grid[x][y].content = EMPTY;
				break;
			case WALL:
				m->grid[x][y].type = WALL;
				m->grid[x][y].content = EMPTY;
				break;
			case TARGET:
				m->grid[x][y].type = TARGET;
				m->grid[x][y].content = EMPTY;
				break;
			case PLAYER:
				m->grid[x][y].type = FLOOR;
				m->grid[x][y].content = PLAYER;
				m->player = (Pair){ x, y };
				break;
			case BOX:
				m->grid[x][y].type = FLOOR;
				m->grid[x][y].content = BOX;
				break;
			default:
				warning("unknown character %c, in map: %d", c, n);
				return NULL;
			}
			x++;
		}
	}
	if (m->player.x == -1 && m->player.y == -1) { // voir pour une egalite plus propre avec macro ?
		warning("no player in map: %d");
		return NULL;
	}
	fclose(fp);
	return m;
}

static Pair
getmapsize(FILE *fp)
{
	Pair size = { 0, 0 };
	int c;
	int len;
	int n;

	len = 0;
	n = 0;
	while ((c = fgetc(fp)) != EOF && c != ';') {
		if (c == '\n' && size.x < len) {
			size.x = len;
			size.y++;
		}
		len++;
		n++;
	}
	/* keep fp unchanged */
	fseek(fp, -n, SEEK_CUR);
	return size;
}

static struct comment *
parsecomment(FILE *fp)
{
	int c;
	int i;
	int opsize, valsize;
	char *op, *val;
	struct comment *cmt;

	cmt = emalloc(sizeof(struct comment));
	cmt->op = PASS;
	c = 0;
	opsize = 16;
	op = emalloc(opsize);
	for (i = 0; (c = fgetc(fp)) >= 0 && c != ' '; i++) {
		if (c == EOF || c == '\n')
			return cmt;
		if (i > opsize - 2)
			op = erealloc(op, opsize * 2);
		op[i] = c;
	}
	if (strlen(op) == 0)
		return cmt;

	c = 0;
	valsize = 16;
	val = emalloc(valsize);
	for (i = 0; (c = fgetc(fp)) && c != '\n' && c != EOF; i++) {
		if (i > valsize - 2)
			val = erealloc(val, valsize * 2);
		val[i] = c;
	}

	if (!strcmp(op, "AUTHOR")) {
		cmt->op = AUTHOR;
		cmt->val.s = emalloc(i);
		strcpy(cmt->val.s, val);
	} else if (!strcmp(op, "COMMENT")) {
		cmt->op = COMMENT;
		cmt->val.s = emalloc(i);
		strcpy(cmt->val.s, val);
	} else if (!strcmp(op, "LEVEL")) {
		cmt->op = LEVEL;
		cmt->val.i = (int)estrtol(val, 10);
	} else {
		cmt->op = PASS;
	}
	free(op);
	free(val);

	return cmt;
}

static int
gotolevel(FILE *fp, int n)
{
	int c;
	struct comment *cmt;

	rewind(fp);
	while ((c = fgetc(fp)) != EOF) {
		if (c == ';') {
			if ((cmt = parsecomment(fp)) == NULL)
				return 1;
			if (cmt->op == LEVEL && cmt->val.i == n)
				return 0;
		}
	}
	return 1;
}
