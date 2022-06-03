#include <stdlib.h>

#include "move.h"
#include "sokoban.h"
#include "util.h"

Stack *
pushstack(Stack **s, Pair move, int boxmoved)
{
	Stack *p;

	if (*s == NULL) {
		*s = emalloc(sizeof(Stack));
		(*s)->move = move;
		(*s)->boxmoved = boxmoved;
		(*s)->prev = NULL;
	} else {
		p = emalloc(sizeof(Stack));
		p->move = move;
		p->boxmoved = boxmoved;
		p->prev = *s;
		s = &p;
	}
	return *s;
}

Stack
popstack(Stack **s)
{
	Stack p;
	Stack *poped;

	p = **s;
	poped = *s;
	*s = (*s)->prev;
	free(poped);

	return p;
}
