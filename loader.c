#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "util.h"
#include "sokoban.h"

enum { PASS, AUTHOR, COMMENT, LEVEL, MAXLEVEL };

struct tag {
	int name;
	char *val;
};

static void	 freetag(struct tag *);
static Pair	 getmapsize(FILE *);
static int	 gotolevel(FILE *, int);
static struct tag	*readtag(FILE *);

void
freemap(Map *m)
{
	int i;

	for (i = 0; i < m->size.x; i++)
		free(m->grid[i]);
	free(m->grid);
	if (m->comment != NULL)
		free(m->comment);
	if (m->author != NULL)
		free(m->author);
	free(m);
}

static void
freetag(struct tag *t)
{
	if (t->val != NULL)
		free(t->val);
	free(t);
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
		if (c == '\n') {
			size.y++;
			if (size.x < len)
				size.x = len;
			len = 0;
		} else
			len++;
		n++;
	}
	/* keep fp unchanged */
	fseek(fp, -(n + 1), SEEK_CUR);
	return size;
}

static int
gotolevel(FILE *fp, int n)
{
	int c;
	struct tag *t;

	rewind(fp);
	while ((c = fgetc(fp)) != EOF) {
		if (c == ';') {
			t = readtag(fp);
			if (t->name == LEVEL && estrtol(t->val, 10) == n) {
				freetag(t);
				return 0;
			}
			freetag(t);
		}
	}
	return 1;
}

Map *
loadmap(char *file, int n)
{
	int c;
	int i;
	int x, y;
	struct tag *t;
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
	m->id = n;
	m->comment = NULL;
	m->author = NULL;
	m->player = (Pair){ -1, -1 };
	m->cursor = (Pair){ -1, -1 };
	/* catch extra tags */
	while ((c = fgetc(fp)) != EOF && c == ';') {

		t = readtag(fp);
		switch (t->name) {
		case AUTHOR:
			m->author = estrdup(t->val);
			break;
		case COMMENT:
			m->comment = estrdup(t->val);
			break;
		case LEVEL:
			/* empty grid, error */
			warning("empty map: %d", n);
			return NULL;
		}
		freetag(t);
	}
	/*
	 * Avoid the first map space to be blown
	 * by the "extra comment" loop condition.
	 */
	if (c != EOF)
		fseek(fp, -1, SEEK_CUR);

	/* fill grid */
	x = 0;
	y = 0;
	m->size = getmapsize(fp);
	if (m->size.x == 0 && m->size.y == 0) {
		warning("empty map: %d", n);
		return NULL;
	}
	m->grid = emalloc(sizeof(Space) * m->size.x);
	for (i = 0; i < m->size.x; i++) {
		m->grid[i] = emalloc(sizeof(Space) * m->size.y);
		memset(m->grid[i], FLOOR, m->size.y);
	}
	while ((c = fgetc(fp)) != EOF && c != ';') {
		if (c == '\n') {
			x = 0;
			y++;
			continue;
		}
		switch (c) {
		case FLOOR:
		case WALL:
		case TARGET:
			m->grid[x][y].type = c;
			m->grid[x][y].content = EMPTY;
			break;
		case '*':
			m->grid[x][y].type = TARGET;
			m->grid[x][y].content = PLAYER;
			m->player = (Pair){ x, y };
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
			warning("unknow character %c, in map: %d", c, n);
			return NULL;
		}
		x++;
	}
	if (m->player.x == -1 && m->player.y == -1) {
		warning("no player in map: %d", n);
		return NULL;
	}
	fclose(fp);
	return m;
}

static struct tag *
readtag(FILE *fp)
{
	int c;
	int i;
	int namesize, valsize;
	char *name, *val;
	struct tag *t;

	t = emalloc(sizeof(struct tag));
	namesize = 8;
	name = emalloc(namesize);
	for (i = 0; (c = fgetc(fp)) && c != EOF && c != ' ' && c != '\n'; i++) {
		if (i > namesize - 2)
			name = erealloc(name, namesize *= 2);
		name[i] = c;
	}
	if (!strcmp(name, "AUTHOR"))
		t->name = AUTHOR;
	else if (!strcmp(name, "COMMENT"))
		t->name = COMMENT;
	else if (!strcmp(name, "LEVEL"))
		t->name = LEVEL;
	else if (!strcmp(name, "MAXLEVEL"))
		t->name = MAXLEVEL;
	else
		t->name = PASS;
	free(name);
	t->val = NULL;
	if (c == ' ') {
		valsize = 8;
		val = emalloc(valsize);
		for (i = 0; (c = fgetc(fp)) && c != EOF && c != '\n'; i++) {
			if (i > valsize - 2)
				val = erealloc(val, valsize *= 2);
			val[i] = c;
		}
		t->val = emalloc(i);
		strcpy(t->val, val);
		free(val);
	}
	return t;
}
