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
			ligne[map->player.x+leftmargin] = '@';
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
