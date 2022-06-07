#include "move.h"

int move (Map *m, Pair move,Stack **s){

    
    int deltax = (*m).player.x + move.x;
    int movey = (*m).player.y + move.y;
    int boxmoved;
    if (canwemove(m,move))
    {
        
        if ((*m).grid[deltax][movey].content == BOX)
        {
			boxmoved = 1;
            (*m).grid[(*m).player.x][(*m).player.y].content = EMPTY;
            (*m).grid[deltax + move.x][movey + move.y].content = BOX;
        }
        else 
        {
            boxmoved = 0;
            (*m).grid[(*m).player.x][(*m).player.y].content = EMPTY;
            
        }
		(*m).player.x = deltax;
        (*m).player.y = movey;
		(*m).grid[deltax][movey].content = PLAYER;
		if (s != NULL)
			pushstack(s, move, boxmoved);
        return 1;
        
    }
    else
    {
        return 0;
    }

}
int canwemove(Map *m, Pair move){

    if ((*m).grid[ (*m).player.x + move.x][(*m).player.y + move.y].type == WALL || ((*m).grid[ (*m).player.x + 2*move.x ][(*m).player.y + 2*move.y ].type == WALL )&& ((*m).grid[ (*m).player.x + move.x][(*m).player.y + move.y].content == BOX ) || (((*m).grid[ (*m).player.x + move.x][(*m).player.y + move.y].content == BOX ) && (((*m).grid[ (*m).player.x + 2*move.x][(*m).player.y +2*move.y].content == BOX ))) ){
        return 0;
    }
    else if ((*m).grid[ (*m).player.x + move.x][(*m).player.y + move.y].content == BOX || (*m).grid[ (*m).player.x + move.x][(*m).player.y + move.y].content == EMPTY   ){
        return 1;
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
			
		}
		}
	}


int loadfromstack(Stack *input,Map *m,Stack *output, int stroke){
	Stack pop;


	if ( input == NULL){
		displaywarning("No move to undo");
		
		return 0;
	}
	else{
		
		while (!popstack(&input, &pop)){
			
			
			displaymap(m,stroke);
			move(m,pop.move,&output);
			stroke++;
			sleep(1);

		}
	
		
		return stroke;
	}

}