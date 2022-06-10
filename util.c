/* See LICENCE file for copyright and licence details */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "input.h"

void
error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void)fprintf(stderr, "sokoban: Error: ");
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	resetterm();
	exit(1);
}

void
warning(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void)fprintf(stderr, "sokoban: Warning: ");
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	resetterm();
	fputc('\n', stderr);
}

void *
emalloc(unsigned int n)
{
	void *p = NULL;

	p = malloc(n);
	if (p == NULL)
		error("malloc");
	memset(p, 0, n);
	return p;
}

void *
erealloc(void *q, unsigned int n)
{
	void *p = NULL;

	p = realloc(q, n);
	if (p == NULL)
		error("realloc");
	return p;
}

char *
estrdup(const char *s1)
{
	char *s2 = NULL;

	s2 = strdup(s1);
	if (s2 == NULL)
		error("strdup");
	return s2;
}

long
estrtol(const char *nptr, int base)
{
	char *endptr = NULL;
	errno = 0;
	long out;

	out = strtol(nptr, &endptr, base);
	if ((endptr == nptr) || ((out == LONG_MAX || out == LONG_MIN)
	   && errno == ERANGE))
		error("invalid integer: %s", nptr);
	return out;
}
