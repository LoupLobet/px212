#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "util.h"
#include "sokoban.h"
#include "move.h"

enum { AUTHOR, COMMENT, LEVEL, PASS };

union value {
	int i;
	char *s;
};

struct comment {
	int op;
	union value val;
};

static void	 freecomment(struct comment *);
static Pair	 getmapsize(FILE *fp);
static int	 gotolevel(FILE *, int);
static struct comment	*parsecomment(FILE *);

static void
freecomment(struct comment *cmt)
{
	free(cmt->val.s);
	free(cmt);
}

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

Map *
loadmap(char *file, int n)
{
	int c;
	int i;
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
	m->id = n;
	m->comment = NULL;
	m->author = NULL;
	m->player = (Pair){ -1, -1 };

	/* catch extra comments */
	while ((c = fgetc(fp)) != EOF && c == ';') {
		cmt = parsecomment(fp);
		switch (cmt->op) {
		case AUTHOR:
			m->author = estrdup(cmt->val.s);
			break;
		case COMMENT:
			m->comment = estrdup(cmt->val.s);
			break;
		case LEVEL:
			/* empty grid, error */
			warning("empty map: %d", n);
			return NULL;
		case PASS:
			break;
		}
		freecomment(cmt);
	}
	/*
	 * avoid the first map space to be blown
	 * by the "extra comment loop" condition
	 */
	fseek(fp, -1, SEEK_CUR);

	/* fill the grid */
	x = 0;
	y = 0;
	m->size = getmapsize(fp);
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
		case PLAYER:
			m->player = (Pair){ x, y };
			/* fallthrough */
		case BOX:
			m->grid[x][y].type = FLOOR;
			m->grid[x][y].content = c;
			break;
		default:
			warning("unknown character %c, in map: %d", c, n);
			return NULL;
		}
		x++;
	}
	if (m->player.x == -1 && m->player.y == -1) {
		warning("no player in map: %d");
		return NULL;
	}
	fclose(fp);
	return m;
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
	for (i = 0; (c = fgetc(fp)) && c != ' '; i++) {
		if (c == EOF || c == '\n') {
			return cmt;
		}
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

int
savemap(Map *m, Stack *s, char *file)
{
	char bufpath[] = "/tmp/bufsokoban";
	Stack pop;
	long int offset;
	int c;
	int i;
	FILE *fp;
	FILE *buf;

	if ((fp = fopen(file, "a+")) == NULL) {
		warning("could not open file: %s", file);
		return 1;
	}
	/* check if a move set is already saved in the file for this map */
	if (gotolevel(fp, m->id)) {
		/* simply append to end of file */
		fseek(fp, 0, SEEK_END);
		fprintf(fp, ";LEVEL %d\n", m->id);
		while (!popstack(&s, &pop))
			fprintf(fp, "%d,%d,%d,", pop.move.x, pop.move.y, pop.boxmoved);
		fputc('\n', fp);
	} else {
		/* edit old move set */
		if ((buf = fopen(bufpath, "w")) == NULL) {
			warning("could not open file: %s", file);
			return 1;
		}
		offset = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		for (i = 0; (c = getc(fp)) != EOF && i < offset; i++)
			fputc(c, buf);
		while ((c = getc(fp)) != EOF && c != '\n');
		while (!popstack(&s, &pop))
			fprintf(buf, "%d,%d,%d,", pop.move.x, pop.move.y, pop.boxmoved);
		fputc('\n', buf);
		while ((c = getc(fp)) != EOF)
			fputc(c, buf);
		fclose(buf);
		rename(bufpath, file);
	}
	fclose(fp);
	return 0;
}
