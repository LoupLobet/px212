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
#include "display.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "display.h"
#include "util.h"

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))
#define savecurspos() printf("\033[s")
#define loadcurspos() printf("\033[u")

static Pair cursorpos = (Pair){-1, -1};
static Pair cursor0 = (Pair){1, 3};

static char spacetochar(Space *s);
int digitnb ( int nb );
static void displaycursor();

static void displaymap(Map* map, int leftmargin)	{
	int maxx = map->size.x, maxy = map->size.y;

	// init ligne
	char* ligne = emalloc(sizeof(char)*maxx + leftmargin + 1);
	for (int i = 0; i < leftmargin; i++) ligne[i] = ' ';

	//remplir lignes
	for (int y = 0; y < maxy; y++) {
		for (int x = 0; x < maxx; x++) {
			ligne[x+leftmargin] = spacetochar(&map->grid[x][y]);
		}
		//display player
		if (map->player.y == y)
			ligne[map->player.x + leftmargin] = '@';
		printf("%s\n", ligne);
		for (int i = 0; i < maxx+1; i++) ligne[i+leftmargin] = 0;
	}
	free(ligne);
}

static void displayheader(Map* map)	{
	int size = map->size.x;
	if (size > (23-4)){
		int b1len = (size - 23 + 4) / 2  + (size-1)%2;
		int b2len = (size - 23 + 4) / 2;
		char *border1 = emalloc(sizeof(char) * (b1len+1));
		memset(border1, '=', b1len);
		char *border2 = emalloc(sizeof(char) * (b2len+1));
		memset(border2, '=', b2len);
		printf("\n//==%sGame of Sokoban==%s\\\\\n\n",border1, border2);
		cursor0 = (Pair){2, 3};
	}	else {
		printf("\n//==Game of Sokoban==\\\\\n\n");
		cursor0 = (Pair){(23-size)/2, 3}; // c degeu de faire calculs et display ici
	}
}




void display(Map* map, int mvnb)
{
	// clear
	printf("\e[1;1H\e[2J");

	// center Map

	displayheader(map);
	// display map
	displaymap(map, cursor0.x);

	// print author if exist and map id
	if (map->author && *map->author)
		printf("\nMap n°%u from %s\n", map->id, map->author);
	else
		printf("\nMap n°%u \n", map->id);

	// print comment if exist
	if (map->comment && *map->comment)
		printf("|\n\\-> %s\n", map->comment);

	// print movement nb
	printf("\nYou made %d strokes\n",mvnb);

	// print help
	// printf("\n - Press 'ESC' to quit \n - Press Arrow Keys to move\n - Press 'z' to undo\n - Press 's' to save\n - Press 'r' to restart\n - Press l nblevel and enter to choose a new level\n - Press 'n' to go to next level\n - Press 'p' to go to previous level\n - Press 'c' to pass in cursor mode and 'ESC' to exit\n");

	printf("\n\n");
}


void displaystr(char *s) {
	printf("\e[1A\e[2K");
	printf("%s\n", s);
	displaycursor();
}


void displaywarning(char *s) {
	printf("\e[1A\e[2K");
	printf("WARNING : %s\n", s);
	displaycursor();
}


/// --- CURSOR --- \\\


static void displaycursor(){
	int column0 = cursor0.x, line0 = cursor0.y;
	int x = cursorpos.x, y = cursorpos.y;
	if(x != -1 && y != -1){
		savecurspos();
		gotoxy(column0+x+1,line0+y+1);
		fflush(stdout);
		loadcurspos();
	} else fflush(stdout);
}


static char spacetochar(Space *s){
	// cas sol (défault)
	char c = ' ';
	//autres cas
	if (s->type == TARGET && s->content == BOX) c = '*';
	else if (s->content == BOX) c = '$';
	else if (s->type == TARGET) c = '.';
	else if (s->type == WALL) c = '#';

	return c;
}


void setcursor(Map *map, Pair pos){
	int px = pos.x, py = pos.y;
	Pair ms = map->size;
	if(px == -1 && py == -1) cursorpos = pos;
	else cursorpos = (Pair){MIN(MAX(pos.x, 0), ms.x), MIN(MAX(pos.y, 0), ms.y)};
	displaycursor();
}


void movecursor(Map *map, Pair mvt){
	setcursor(map, (Pair){cursorpos.x + mvt.x, cursorpos.y + mvt.y});
}


Pair getcursor() {return cursorpos;}

void error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void)fprintf(stderr, "sokoban: Error: ");
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	exit(1);
}

void warning(const char *fmt, ...)
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

long estrtol(const char *nptr, int base)
{
	char *endptr = NULL;
	errno = 0;
	long out;

	out = strtol(nptr, &endptr, base);
	if ((endptr == nptr) || ((out == LONG_MAX || out == LONG_MIN) && errno == ERANGE))
		error("invalid integer: %s", nptr);
	return out;
}

FILE *
efopen(const char *path, const char *mode)
{
	FILE *fp;

	if ((fp = fopen(path, mode)) == NULL)
		error("could not open file: %s", path);
	return fp;
}

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

int io(void)
{

	int key;

	char r[MAXSIZE];
	int c;

	if ((c = read(0, r, 3)) == -1)
		return false;

	switch (r[0])
	{
	case 27:
		if ((c == 3) && (r[1] == 91))
		{
			key = (r[2] - 64);
			switch (key)
			{

			case UP:
				return 'U';
				break;
			case DOWN:
				return 'D';
				break;
			case LEFT:
				return 'L';
				break;
			case RIGHT:
				return 'R';
				break;
			default:
				return 'P';
				break;
			}
		}
		else if (c == 1 && r[0] == 27)
			return 27;
		else
			return false;
		break;
	default:
		return r[0];
		break;
	}
}

int configureTerminal()
{
	struct termios new;
	if (tcgetattr(0, &new) == -1)
	{
		perror("tcgetattr");
		return -1;
	}
	new.c_lflag &= ~(ICANON); // Met le terminal en mode canonique.
	new.c_lflag &= ~(ECHO);	  // les touches tapées ne s'inscriront plus dans le terminal
	new.c_cc[VMIN] = 1;
	new.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSANOW, &new) == -1)
	{
		perror("tcsetattr");
		return false;
	}
	return true;
}

void listeAffiche(Stack *ptr)
{
	if (NULL == ptr)
		printf("Liste vide!");
	else
		printf("Contenu de la liste : ");
	while (NULL != ptr)
	{
		printf("\n Boite bouge : %d ", ptr->boxmoved);
		printf("\n mouvement : (%d,%d )\n", ptr->move.x, ptr->move.y);
		ptr = ptr->prev;
	}
	printf("\n");
}

int move (Map *m, Pair move,Stack **s){

    
    int movex = (*m).player.x + move.x;
    int movey = (*m).player.y + move.y;
    int boxmoved;
    if (canwemove(m,move))
    {
        
        if ((*m).grid[movex][movey].content == BOX)
        {
			boxmoved = 1;
            (*m).grid[(*m).player.x][(*m).player.y].content = EMPTY;
            (*m).grid[movex + move.x][movey + move.y].content = BOX;
        }
        else 
        {
            boxmoved = 0;
            (*m).grid[(*m).player.x][(*m).player.y].content = EMPTY;
            
        }
		(*m).player.x = movex;
        (*m).player.y = movey;
		(*m).grid[movex][movey].content = PLAYER;
		if (s != NULL)
			pushstack(s, move, boxmoved);
			
        return 1;
        
    }
    else
    {
        return 0;
    }

}
int canwemove(Map *m, Pair move)
{

	if (m->grid[m->player.x + move.x][m->player.y + move.y].type == WALL || (m->grid[m->player.x + 2 * move.x][m->player.y + 2 * move.y].type == WALL) && (m->grid[m->player.x + move.x][m->player.y + move.y].content == BOX) || ((m->grid[m->player.x + move.x][m->player.y + move.y].content == BOX) && ((m->grid[m->player.x + 2 * move.x][m->player.y + 2 * move.y].content == BOX))))
	{
		return 0;
	}
	else if (m->grid[m->player.x + move.x][m->player.y + move.y].content == BOX || m->grid[m->player.x + move.x][m->player.y + move.y].content == EMPTY)
	{
		return 1;
	}
}

Stack *pushstack(Stack **s, Pair move, int boxmoved)
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

	
	Stack p;
	if ( s == NULL){
		displaywarning("No movement save !");
		return EXIT_FAILURE;
	}
	else {
	
	
		popstack(s,&p);
	
		
		m->grid[m->player.x][m->player.y].content = EMPTY;
	
		m->player.x -=  p.move.x;
		m->player.y -= p.move.y;
		m->grid[m->player.x][m->player.y].content = PLAYER;
		if (p.boxmoved == 1) {
			m->grid[m->player.x + p.move.x][m->player.y + p.move.y].content = BOX;
			m->grid[m->player.x + 2*(p.move.x)][m->player.y + 2*(p.move.y)].content = EMPTY;
			
		}
		
		}
		
	}



int loadfromstack(Stack *input, Map *m, Stack **output, int stroke)
{
	Stack pop;

	if (input == NULL)
	{
		displaywarning("No move to undo");

		return 0;
	}
	else
	{

		while (!popstack(&input, &pop))
		{

			
			move(m, pop.move, output);
			
			
			
		}

		return stroke;
	}
}

Map *loadingmap(Map *m, int level, char *file, Stack **s)
{
	Stack *p;
	char choose;
	m = loadmap(file, level);
	if (m == NULL)
	{
		displaywarning("Could not load map");
		error("could not load map");

	}
	
	if ((p=loadsave(level,file)) != NULL)
	{
		displaystr("\nA save is detected, do you want to load it (y\\n) ? \n");
		if ((choose = io()) == 'y')
		{

			 loadfromstack(p,m,s,0);

		}
		
	}
	return m;
	
	
}

int countstrokes(Stack **s){
	Stack *p;
	int i = 0;
	if (NULL == s)
		return i;
	else{
		
		while (NULL != *s)
		{
			i++;
			p = *s;
			*s = p->prev;
			
		}
	}
	return i;
}



Map *initialisation(Map *m, char *file, int level, Stack **s)
{
	int stroke;
	m = loadingmap(m, level, file,s);
	listeAffiche(*s);
	stroke = countstrokes(s);
	display(m, stroke);

	return m;
}

int verifiegagne(Map *m)
{
	int i, j;
	for (j = 0; j < m->size.y; j++)
	{
		for (i = 0; i < m->size.x; i++)
		{
			if (m->grid[i][j].type == TARGET && m->grid[i][j].content != BOX)
			{
				return 0;
			}
		}
	}
	return 1;
}

int nextlevel(Map *m, int level, Stack **s)
{
	int stroke = 0;
	char *file = "levels.lvl";
	freemap(m);
	level++;
	m = loadingmap(m, level, file,s);
	stroke = countstrokes(s);

	return stroke;
}

int prevlevel(Map *m, int level,Stack **s)
{
	char *file = "levels.lvl";
	int stroke = 0;
	if (level == 1)
	{
		printf("You are on the first level !\n");
		return 0;
	}

	
	
	freemap(m);
	level--;
	m = loadingmap(m, level, file,s);
	stroke = countstrokes(s);
	return stroke;
}

int changelevel(Map *m, Stack **s)
{
	int stroke = 0;
	int size = 0;
	int n = 0;
	int niveau[10] = {-1};
	char temp;
	char *file = "levels.lvl";
	while ((temp = io()) != 10)
	{
		niveau[n] = temp - '0';
		n++;
	}

	size = n;
	int i = 0;
	int level = 0;
	for (i = 0; i < size; i++)
		level = 10 * level + niveau[i];

	
	freemap(m);
	m = loadingmap(m, level, file,s);
	return level;
}

int restart(Map *m, int level, Stack **s)
{
	int stroke = 0;
	char *file = "levels.lvl";
	freemap(m);
	m = loadingmap(m, level, file,s);
	return stroke;
}



int undomovement(Map *m, Stack **s, int stroke)
{
	if (stroke == 0)
	{
		displaystr("You can't undo anymore !\n");
		
		return 0;
	}
	stroke--;

	undomove(s, m);
	return stroke;
}

void cursormove(Map *m)
{
	char mov;
	setcursor(m, (Pair){m->player.x, m->player.y});
	displaycursor();
	while ((mov = io()) != 27)
	{

		switch (mov)
		{
		case 'U':
			movecursor(m, (Pair){0, -1});
			break;
		case 'D':
			movecursor(m, (Pair){0, 1});
			break;
		case 'L':
			movecursor(m, (Pair){-1, 0});
			break;
		case 'R':
			movecursor(m, (Pair){1, 0});
			break;
		default:
			displaywarning("Invalid move");
			break;
		}
		displaycursor();
	}
}

int main(int argc, char *argv[])
{

	Map *m;
	Stack *s;
	Stack *p;
	int choix;
	int stroke = 0;
	int level = 1;
	int tmp;
	char temp;
	char *file;
	int i = 0;
	char mystr[30];
	char soluce[80];
	char mov;

	if (!configureTerminal())
	{
		displaywarning("Could not configure terminal");
		error("Configuration of the terminal is impossible");
		return 0;
	}

	if (argc == 3)
	{

		m = initialisation(m, argv[1], atoi(argv[2]),&s);
	}
	else if (argc == 2)
	{
		m = initialisation(m, argv[1], 1,&s);
	}
	else if (argc == 1)
	{
		m = initialisation(m, "levels.lvl", 1,&s);
	}
	else
	{
		displaywarning("Wrong number of arguments");
		error("Problem of argument : Arguments must be ./prog file NbLevel");
		return EXIT_FAILURE;
	}

	while ((mov = io()) != 27)
	{

		switch (mov)
		{
		case 'U':
			if (move(m, (Pair){0, -1}, &s))
				stroke++;
			break;
		case 'D':
			if (move(m, (Pair){0, 1}, &s))
				stroke++;
			break;
		case 'L':
			if (move(m, (Pair){-1, 0}, &s))
				stroke++;
			break;
		case 'R':
			if (move(m, (Pair){1, 0}, &s))
				stroke++;
			break;
		case 'z':
			stroke = undomovement(m, &s, stroke);
			break;
		case 'r':
			stroke = restart(m, level,&s);
			break;
		case 's':
			file = "levels.lvl";
			savemap(m,s,file);
			return 0;
		case 'l':
			level = changelevel(m,&s);
			stroke = countstrokes(&s);
			break;
		case 'p':
			prevlevel(m, level,&s);
			level--;
			break;
		case 'n':
			stroke = nextlevel(m, level,&s);
			level++;
			break;
		case 'c':
			cursormove(m);
			continue;
			break;
		default:
			displaywarning("Wrong input");
			error("Vous n'avez pas entrez la bonne touche");
			return 0;
			break;

			//			for (i = 0; i < ncommands; i++) {
			//				if (commands[i] = choix)
			//					(*commands[i].func)(m, s, Args);
			//			}
		}

		display(m, stroke);

		if (!verifiegagne(m))
		{
			continue;
		}
		else

		tmp = stroke + 1;
		stroke = 0;
		file = "levels.lvl";
		//sprintf(mystr, "%d", level); // convert level to string
		
		// savemap(m, s, soluce);
		level++;
		freemap(m);
		m = loadingmap(m, level, file,&s);
		stroke = countstrokes(&s);
		display(m, stroke);
		printf("\nYou win level %d in %d strokes , you are level %d\n", level, tmp, level--);
	}
	return 0;
}


//  We choose at the beginning to load a save or show a solution
