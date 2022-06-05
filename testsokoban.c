#include <stdio.h>
#include <stdlib.h> 
#include "loader.h"
#include "move.h"
#include <string.h>
#include "util.h"
#include "input.h"
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>




void
error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void)fprintf(stderr, "sokoban: Error: ");
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
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






int io(void) {

  int key;

  char r[MAXSIZE];
	int c;  

	if ((c=read(0,r,3)) == - 1 ) return false; 

	switch (r[0]) {
	case 27 :  
      if ((c==3) && (r[1]==91)){
        key = (r[2]-64);
        switch (key) {
    
        case UP: return 'U'; break;
        case DOWN: return 'D'; break;
        case LEFT: return 'L'; break;
        case RIGHT:return 'R'; break;
        default:  return 'P'; break;
        }
      }  
      else if (c == 1 && r[0] == 27)
        return 27; 
	  else
	  	return false;
      break; 
	default: return r[0]; break; 
	}
				
          
      
}
   
 





int configureTerminal()
{
	struct termios new;
	if (tcgetattr(0,&new)==-1) {
      		perror("tcgetattr");
		return -1;
    	}
	new.c_lflag &= ~(ICANON); // Met le terminal en mode canonique.
	new.c_lflag &= ~(ECHO);   // les touches tapées ne s'inscriront plus dans le terminal
	new.c_cc[VMIN]=1;
	new.c_cc[VTIME]=0;
	if (tcsetattr(0,TCSANOW,&new)==-1) {
      		perror("tcsetattr");
		return false;
    	}
	return true;
}








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





void listeAffiche(Stack * ptr){
	if ( NULL == ptr )
		printf("Liste vide!") ;
	else 
		printf("Contenu de la liste : ") ;
	while ( NULL != ptr ) 	{
		printf("\n Boite bouge : %d ",ptr->boxmoved);
		printf("\n mouvement : (%d,%d )\n",ptr->move.x,ptr->move.y);
		ptr = ptr->prev ;
		}
	printf("\n") ;
	}

int verifiegagne(Map *m){
	int i,j;
	for (j = 0; j < m->size.y; j++) {
		for (i = 0; i < m->size.x; i++) {
			if (m->grid[i][j].type == TARGET && m->grid[i][j].content != BOX){
				return 0;
			}
		}
	}
	return 1;
}



Map* loadingmap(Map *m, int level, char *file){
	m = loadmap(file, level);
	if (m == NULL) {
		error("could not load map");
	}
	return m;

}

void displaytemp(Map *m, int stroke){
	int i, j;
	printf("\e[1;1H\e[2J");
	printf("\nYou made %d strokes\n",stroke);
	for (j = 0; j < m->size.y; j++) {
		for (i = 0; i < m->size.x; i++) {
			if (m->grid[i][j].type == TARGET && m->grid[i][j].content == BOX)
				putchar('*');
			else if (m->grid[i][j].content != EMPTY)
				putchar(m->grid[i][j].content);
			else
				putchar(m->grid[i][j].type);
		}
		putchar('\n');
	}
	printf("\n - Press 'ESC' to quit \n - Press Arrow Keys to move\n - Press 'z' to undo\n - Press 's' to save\n - Press 'r' to restart\n");
	
}

Map* initialisation(Map *m){
	printf("\n=======Game of Sokoban===========\n");
	printf("\nPress 'a' to load an old save\n");
	printf("\nPress 'b' to load a new game\n\n\n");
	int choose = io();
	if (choose == 'a'){
		char *file = "levels.save";
		m = loadingmap(m,0,file);
	}
	else if (choose == 'b'){
		char *file = "levels.lvl";
		m = loadingmap(m,1,file);
		displaytemp(m,0);
	}
	else{
		printf("\nWrong choice\n");
		exit(EXIT_FAILURE);
	}
	return m;

}


int main(int argc, char *argv[])
{

	Map *m;
	Stack *s;
	int choix ;
	int stroke = 0;
	int level = 1;

	if (!configureTerminal())
	{
		error("Configuration of the terminal is impossible");
		return 0;
	}
	
	

	m = initialisation(m);

	
    while ((choix = whichmove(m,&s)) != 27){
		if (!verifiegagne(m)){
		
			stroke ++;
			if (choix == 'z') {
				stroke -=2;
				undomove(&s,m);
			}
			else if (choix == 's')
			{
				savemap(m, s, "levels.save");
				return 0;
			}
			else if (choix == 'r'){
				stroke = 0;
				char *file = "levels.lvl";
				m = loadingmap(m, level,file);
			}
			displaytemp(m,stroke);
		
		}
		else{// TODO: make display function and save solution
			displaytemp(m,stroke);
			
			level ++;
			int tmp = stroke +1 ;
			stroke = 0;
			char *file = "levels.lvl";
			m = loadingmap(m,level,file);
			displaytemp(m,stroke);
			printf("\nYou win level %d in %d strokes , you are level %d\n",level,tmp, level--);

		}
	}
		

	return 0;
    
}

	
