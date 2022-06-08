#include "move.h"


/**
 * @brief This function allow the user to move the player in the map.
 * For that we verify first if we can move with canwemove() and then
 * we manage each case when we can do a movement.
 * Different cases are : we can move with a box in front of us. So in
 * this case, we move the player and the box and we specify that a box
 * moved to save it + the move in a stack (s).
 * The other case is : we can move and in front of us they are nothing.
 * So , we simply move the player and specify that no box move to save
 * it in the stack s.
 * @param m Map
 * @param move Move that we want to do
 * @param s Stack of movement
 * @return int 1 if the player has been moved, 0 otherwise.
 */
int
move (Map *m, Pair move,Stack **s)
{
    Pair targ = { (*m).player.x + move.x, (*m).player.y };
    Pair player = { (*m).player.x, (*m).player.y };
    Pair behind = { (*m).player.x + 2 * move.x, (*m).player.y + 2 * move.y };
    int boxmoved;

    if (canmove(m,move)) {
        if ((*m).grid[targ.x][targ.x].content == BOX) {
        	boxmoved = 1;
            (*m).grid[behind][behind].content = BOX;
        } else {
            boxmoved = 0;
        (*m).grid[player.x][player.y].content = EMPTY;
		player.x = targ.x;
        player.y = targ.y;
		(*m).grid[targ.x][targ.y].content = PLAYER;
		if (s != NULL)
			pushstack(s, move, boxmoved);
        return 1;
    }
    return 0;
}

/**
 * @brief This function is used to verify if we can do the movement
 * that the user want to do. For that, the function return 1 if we
 * can and 0 if we can not.
 * The cases when we cannot are : we have
 * two box in front of us / we have a wall in front of us/ we want
 * to move a box but they are a wall in front of the box.
 * The cases when we can after we verify if we cannot are: we want
 * to move a box/ we want to move in a empty space.
 * @param m Map
 * @param move Move that we want to do
 * @return int 1 if we can move / 0 if we can't move
 */
int
canmove(Map *m, Pair move)
{
	Pair targ = { m->player.x + move.x, m->player.y + move.y };
	Pair behind = { m->player.x + 2 * move.x, m->player.y + 2 * move.y };

	if (m->grid[targ][targ].type != WALL || m->grid[targ][targ].content == BOX &&
	    m->grid[behind][behind].type != WALL)
	{
		return 1;
	} else
		return 0;
}

/**
 * @brief Pushes an element to the stack
 * @param s Stack of movement
 * @param move Move that we want to save
 * @param boxmoved 1 if a box was moved 0 else
 * @return Stack* return a pointer of the new stack
 */
Stack
*pushstack(Stack **s, Pair move, int boxmoved)
{
	Stack *p;

	if (*s == NULL)
	{
		*s = emalloc(sizeof(Stack));
		(*s)->move = move;
		(*s)->boxmoved = boxmoved;
		(*s)->prev = NULL;
	}
	else
	{
		p = emalloc(sizeof(Stack));
		p->move = move;
		p->boxmoved = boxmoved;
		p->prev = *s;
		*s = p;
	}
	return *s;
}

/**
 * @brief This function pop the last element of the stack s in the stack
 * pop to use it after. We return 1 if they are a problem and 0 else.
 * @param s stack
 * @param pop poped element
 * @return int 0 if the stack has been poped (wasn't empty), 1 otherwise.
 */
int
popstack(Stack **s, Stack *pop)
{
	Stack *p;

	if (*s == NULL)
		return 1;
	*pop = **s;
	p = *s;
	*s = p->prev;
	free(p);

	return 0;
}

/**
 * @brief This function allows the user to undo his last movement.
 * For that, we use the stack s where we save all the movement and
 * we second stack p. We pop the last element of s and we place it
 * in p. When this is done, we undo the movement by using which
 * movement it was, the position of the player save in the structure
 * of the map and the fact that a box move or not. With that, we use
 * the same algorithm as move by "inversing" it.
 * @param s stack
 * @param m map
 * @return int 0 if a move has been undone, 1 otherwise
 */
int
undomove(Stack **s, Map *m)
{
	Stack pop;
	Pair player = { m->player.x, m->player.y };

	if (popstack(s,&pop)) {
		displaywarning("No move to undo !");
		return 1;
	}
	m->grid[player.x][player.y].content = EMPTY;
	player.x -= pop.move.x;
	player.y -= pop.move.y;
	m->grid[player.x][player.y].content = PLAYER;
	if (p.boxmoved == 1) {
		m->grid[player.x + pop.move.x][player.y + pop.move.y].content = BOX;
		m->grid[player.x + 2*(pop.move.x)][player.y + 2*(pop.move.y)].content = EMPTY;
	}
	return 0;
}

/**
 * @brief This function can be used to show the solution to the user
 * or for the Artificial Intelligence by doing automatically the
 * movement that are save in the stack input. For that, if input is
 * not null , we simply pop the last element of the stack so the first
 * move to do and we use move to do it. Of course, we do a move so we
 * increase the counter because the function return it. We do a sleep
 * of 1 second to show to the user each move independantly.
 * @param input Stack of movement that we want to do
 * @param m Map
 * @param output Stack of movement of the level
 * @param stroke Number of strokes
 * @return void
 */
void
stack(Stack *input, Map *m, Stack **output)
{
	Stack pop;

	if (input == NULL)
		displaywarning("No move to do");
	else {
		while (!popstack(&input, &pop))
			move(m, pop.move, output);
	}
}

/**
 * @brief This function allows the user to move the cursor placed
 * first at the player position when he pass to cursor mode.This
 * function is the beginning for advanced functions when we want
 * to specify a destination to go.
 * @param m Map
 * @return void
 */
void cursormove(Map *m)
{
	char move;
	setcursor(m, (Pair){m->player.x, m->player.y});
	displaycursor();
	while ((move = io()) != 27) {
		switch (move) {
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
		default:
			displaywarning("Invalid move");
		}
		displaycursor();
	}
}