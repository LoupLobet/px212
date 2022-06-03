#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "util.h"

enum { AUTHOR, COMMENT, LEVEL, MAXLEVEL, PASS };

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
static int	 gotolevel(FILE *, int);

struct lvl *table;

int
main(int argc, char *argv[])
{
	FILE *fp;
	int c;

	if ((fp = fopen("levels.lvl", "r")) == NULL)
		error("could not open file: %s\n", argv[1]);
	if (gotolevel(fp, estrtol(argv[1], 10)))
		error("no such level: %s\n", argv[1]);
	while ((c = fgetc(fp))!= EOF) {
		if (c == ';')
			break;
		putchar(c);
	}
}

struct comment *
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
	} else if (!strcmp(op, "MAXLEVEL")) {
		cmt->op = MAXLEVEL;
		cmt->val.i = (int)estrtol(val, 10);
	} else {
		cmt->op = PASS;
	}
	free(op);
	free(val);

	return cmt;
}

int
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
