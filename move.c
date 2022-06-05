


int move (Map *m, Pair move,Stack **s){

    
    int deltax = (*m).player.x + move.x;
    int deltay = (*m).player.y + move.y;
    int boxmoved;
    if (canwemove(m,(Pair){deltax, deltay},move))
    {
        
        if ((*m).grid[deltax][deltay].content == BOX)
        {
            (*m).grid[(*m).player.x][(*m).player.y].content = EMPTY;
            (*m).grid[deltax + move.x][deltay + move.y].content = BOX;
        }
        else 
        {
            boxmoved = 0;
            (*m).grid[(*m).player.x][(*m).player.y].content = EMPTY;
            
        }
		(*m).player.x = deltax;
        (*m).player.y = deltay;
		(*m).grid[deltax][deltay].content = PLAYER;
		if (s != NULL)
			pushstack(s, move, boxmoved);
        return 1;
        
    }
    else
    {
        return 0;
    }

}
int canwemove(Map *m, Pair movepl, Pair movebox){
    if ((*m).grid[movepl.x][movepl.y].type == WALL || ((*m).grid[movepl.x + movebox.x][movepl.y + movebox.y].type == WALL )&& ((*m).grid[movepl.x][movepl.y].content == BOX ) ){
        return 0;
    }
    else if ((*m).grid[movepl.x][movepl.y].content == BOX || (*m).grid[movepl.x][movepl.y].content == EMPTY   ){
        return 1;
    }
   
	
}


int whichmove(Map *m,Stack **s){
    char mov;
    mov = io();
    switch (mov) {
        case 'U': move (m,(Pair){0,-1},s); break;
        case 'D': move (m,(Pair){0,1},s); break;
        case 'L': move (m,(Pair){-1,0},s); break;
        case 'R': move (m,(Pair){1,0},s); break;
        case 27: return 27;
		case 'z': return 'z';
		case 'r': return 'r';
		case 's': return 's';
        default: break;
    }

}


Stack * pushstack(Stack **s, Pair move, int boxmoved)
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
		*s = p;
	}
	return *s;
}

int popstack(Stack **s, Stack *pop)
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






int undomove(Stack **s, Map *m){

	Stack *p;

	if ( *s == NULL){
		printf("Aucun mouvement enregistré !") ;
		return EXIT_FAILURE;
	}
	else {
	
		
		p = *s;
		*s = p->prev;
		
		(*m).grid[(*m).player.x][(*m).player.y].content = EMPTY;
	
		(*m).player.x -=  p->move.x;
		(*m).player.y -= p->move.y;
		(*m).grid[(*m).player.x][(*m).player.y].content = PLAYER;
		if (p->boxmoved == 1) {
			(*m).grid[(*m).player.x + p->move.x][(*m).player.y + p->move.y].content = BOX;
			(*m).grid[(*m).player.x + 2*(p->move.x)][(*m).player.y + 2*(p->move.y)].content = EMPTY;
			(*m).grid[(*m).player.x + 2*(p->move.x)][(*m).player.y + 2*(p->move.y)].content = FLOOR;
		}
		}
	}
