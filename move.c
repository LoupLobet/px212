#include "move.h"

//int move (Map *m, int largeur,Stack **movement, int longueur){
//
//
//    int deplacementJoueurEnLargeur = (*m).player.x + largeur;
//    int deplacementJoueurEnLongueur = (*m).player.y + longueur;
//    int boxmoved;
//    int result = canwemove(m,deplacementJoueurEnLargeur, deplacementJoueurEnLongueur);
//    int result2;
//    if (result == 1 )
//    {
//
//        result2 = canwemove(m, deplacementJoueurEnLargeur + largeur, deplacementJoueurEnLongueur + longueur);
//        if (result2 ==1)
//        {
//            return 0;
//        }
//        else if (result2 == 2) {
//			boxmoved = 1;
//        	(*m).grid[(*m).player.x][(*m).player.y].content = EMPTY;
//            (*m).player.x = deplacementJoueurEnLargeur;
//            (*m).player.y = deplacementJoueurEnLongueur;
//            (*m).grid[deplacementJoueurEnLargeur ][deplacementJoueurEnLongueur].content = PLAYER;
//            (*m).grid[deplacementJoueurEnLargeur + largeur][deplacementJoueurEnLongueur + longueur].content = BOX;
//
//        }
//        else{
//            return 0;
//        }
//
//
//
//    }
//    else if (result ==2){
//
//            boxmoved = 0;
//            (*m).grid[(*m).player.x][(*m).player.y].content = EMPTY;
//            (*m).player.x = deplacementJoueurEnLargeur;
//            (*m).player.y = deplacementJoueurEnLongueur;
//
//            (*m).grid[deplacementJoueurEnLargeur][deplacementJoueurEnLongueur].content = PLAYER;
//
//        }
//    else{
//        return 0;
//    }
//
//
//
//    pushstack(movement,(Pair){largeur,longueur},boxmoved);
//    return 0;
//}
//
//
//int which_move(Map *m,Stack **movement){
//    char mov;
//    mov = io();
//    switch (mov) {
//        case 'U': move (m,0,movement,-1); break;
//        case 'D': move (m,0,movement,1); break;
//        case 'L': move (m,-1,movement,0); break;
//        case 'R': move (m,1,movement,0); break;
//        case 27: return 27;
//		case 'z': return 'z';
//		case 'r': return 'r';
//		case 's': return 's';
//        default: break;
//    }
//
//}

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
		*s = p;
	}
	return *s;
}

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

int
canmove(Map *m, Pair move)
{
	Space playerdst;
	Space boxdst;

	playerdst.type = m->grid[m->player.x + move.x][m->player.y + move.y].type;
	playerdst.content = m->grid[m->player.x + move.x][m->player.y + move.y].content;
	if (playerdst.type == WALL)
		return 1;
	if (playerdst.content == BOX) {
		boxdst.type = m->grid[m->player.x + 2 * move.x][m->player.y + 2 * move.y].type;
		boxdst.content = m->grid[m->player.x + 2 * move.x][m->player.y + 2 * move.y].content;
		if (boxdst.type == WALL || boxdst.content == BOX)
			return 1;
	}
	return 0;
}

int
domove(Map *m, Pair move, Stack **s)
{
	int boxmoved;

	if (!canmove(m, move)) {
		boxmoved = 0;
		if (m->grid[m->player.x + move.x][m->player.y + move.y].content == BOX) {
			m->grid[m->player.x + 2 * move.x][m->player.y + 2 * move.y].content = BOX;
			boxmoved = 1;
		}
		m->grid[m->player.x][m->player.y].content = EMPTY;
		m->grid[m->player.x + move.x][m->player.y + move.y].content = PLAYER;
		m->player.x = m->player.x + move.x;
		m->player.y = m->player.y + move.y;
		if (s != NULL)
			pushstack(s, move, boxmoved);
		return 1;
	}
	return 0;
}

//int canwemove(Map *m,int x , int y){
//    if ((*m).grid[x][y].type == WALL ){
//        return 0;
//    }
//    else if ((*m).grid[x][y].content == BOX  ){
//        return 1;
//    }
//    else if ((*m).grid[x][y].content == EMPTY)
//    {
//        return 2;
//
//    }
//
//}

//int undomove(Stack **movement, Map *m){
//
//	Stack *p;
//
//	if ( *movement == NULL){
//		printf("Aucun mouvement enregistré !") ;
//		return EXIT_FAILURE;
//	}
//	else {
//
//
//		p = *movement;
//		*movement = p->prev;
//
//		(*m).grid[(*m).player.x][(*m).player.y].content = EMPTY;
//
//		(*m).player.x -=  p->move.x;
//		(*m).player.y -= p->move.y;
//		(*m).grid[(*m).player.x][(*m).player.y].content = PLAYER;
//		if (p->boxmoved == 1) {
//			(*m).grid[(*m).player.x + p->move.x][(*m).player.y + p->move.y].content = BOX;
//			(*m).grid[(*m).player.x + 2*(p->move.x)][(*m).player.y + 2*(p->move.y)].content = EMPTY;
//			(*m).grid[(*m).player.x + 2*(p->move.x)][(*m).player.y + 2*(p->move.y)].content = FLOOR;
//		}
//		}
//	}
