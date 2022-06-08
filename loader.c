#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "util.h"
#include "sokoban.h"
#include "move.h"

enum { PASS, AUTHOR, COMMENT, LEVEL, MAXLEVEL, SAVE };

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
loadmap(char *file, int id)
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
	if (gotolevel(fp, id)) {
		warning("no such level: %s", file);
		return NULL;
	}
	m = emalloc(sizeof(Map));
	m->id = id;
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
			warning("empty map: %d", id);
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
		warning("empty map: %d", id);
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
			warning("unknow character %c, in map: %d", c, id);
			return NULL;
		}
		x++;
	}
	if (m->player.x == -1 && m->player.y == -1) {
		warning("no player in map: %d", id);
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
	else if (!strcmp(name, "SAVE"))
		t->name = SAVE;
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
		val[i] = '\0';
		t->val = emalloc(i);
		strcpy(t->val, val);
		free(val);
	}
	return t;
}

int
savemap(Map *m, Stack *s, char *file)
{
	char template[] = "/tmp/sokoban.XXXXXX";
	Stack *pop;
	Pair move;
	long headpos, tailpos;
	int c;
	int i;
	int fd;
	FILE *rfp, *wfp;
	struct tag *t;
	char *temp;

	if ((temp = mktemp(template)) == NULL) {
		warning("impossible to create temp file");
		return 1;
	}
	/* race condition here */
	if ((wfp = fopen(temp, "w")) == NULL) {
		warning("couldn't open temp file");
		return 1;
	}
	if ((rfp = fopen(file, "r")) == NULL) {
		warning("couldn't open file: %s", file);
		return 1;
	}

	if (gotolevel(rfp, m->id)) {
		warning("no such level %d in file: %s", m->id, file);
		return 1;
	}
	headpos = ftell(rfp);
	/* check for an already existing ;SAVE tag */
	while ((c = fgetc(rfp)) != EOF && c == ';') {
		t = readtag(rfp);
		if (t->name == SAVE) {
			freetag(t);
			break;
		}
		headpos = ftell(rfp);
		freetag(t);
	}
	/* Avoid the first map space to be blowb by the loop condition. */
	if (c != EOF)
		fseek(rfp, -1, SEEK_CUR);
	tailpos = ftell(rfp);
	rewind(rfp);

	/* head */
	for (i = 0; i < headpos; i++) {
		c = fgetc(rfp); /* no EOF */
		(void)fputc(c, wfp);
	}
	/* save */
	(void)fprintf(wfp, ";SAVE ");
	for (pop = s; pop != NULL; pop = pop->prev) {
		move.x = pop->move.x;
		move.y = pop->move.y;
		if (pop->move.x == -1)
			move.x = 2;
		if (pop->move.y == -1)
			move.y = 2;
		(void)fprintf(wfp, "%d,%d,%d,", move.x, move.y, pop->boxmoved);
	}
	(void)fputc('\n', wfp);
	/* tail */
	fseek(rfp, tailpos + 1, SEEK_SET);
	while ((c = fgetc(rfp)) != EOF)
		(void)fputc(c, wfp);
	fclose(rfp);
	fclose(wfp);
	close(fd);
	rename(temp, file);
	return 0;
}

Stack *
loadsave(int id, char *file)
{
	Stack pop;
	Pair move;
	int boxmoved;
	int c;
	FILE *fp;
	char *p;
	Stack *s;
	struct tag *t;

	if ((fp = fopen(file, "r")) == NULL) {
		warning("couldn't open file: %s", file);
		return NULL;
	}
	if (gotolevel(fp, id))
		return NULL;

	while ((c = fgetc(fp)) != EOF && c == ';') {
		t = readtag(fp);
		if (t->name == SAVE) {
			/*
			 * We makes assumption that t->val is a non ill formed save
			 * (comma separated digits list, with a multiple of 3 length).
			 */
			p = t->val;
			s = NULL;
			while(strlen(p)) {
				move.x = p[0] - 48 - 3 * (p[0] == '2');
				move.y = p[2] - 48 - 3 * (p[2] == '2');
				boxmoved = p[4] - 48;
				s = pushstack(&s, move, boxmoved);
				p += 6;
			}
			freetag(t);
			return s;
		}
		freetag(t);
	}
	return NULL;
}
