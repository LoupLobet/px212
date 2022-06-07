#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "display.h"
#include "util.h"

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))
#define savecurspos() printf("\033[s")
#define loadcurspos() printf("\033[u")

static Pair cursorpos;

static char spacetochar(Space *s);
int digitnb ( int nb );
static void displaycursor();


void display(Map* map, int mvnb)
{
	// clear
	printf("\e[1;1H\e[2J");
	// print comment if exist
	if (map->comment && *map->comment)
		printf("--> %s <--\n\n", map->comment);
	else
		printf("\n\n");
	// display map
	int maxx = map->size.x, maxy = map->size.y;
	char* ligne = emalloc(sizeof(char)*maxx+1);
	for (int y = 0; y < maxy; y++) {
		for (int x = 0; x < maxx; x++) {
			ligne[x] = spacetochar(&map->grid[x][y]);
		}
		//display player
		if (map->player.y == y)
			ligne[map->player.x] = '@';
		printf("%s\n", ligne);
		for (int i = 0; i < maxx+1; i++) ligne[i] = 0;
	}
	free(ligne);
	// print author if exist and map id
	if (map->author && *map->author)
		printf("\nMap n°%u from %s\n", map->id, map->author);
	else
		printf("\nMap n°%u \n", map->id);
	// print movement nb
	printf("\nYou made %d strokes\n",mvnb);
}


void displaystr(char *s) {
	printf("\n%s\n", s);
	displaycursor();
}


void displaywarning(char *s) {
	int l = strlen(s);
	char *border = emalloc(l);
	memset(border, '=', l-1);
	border[l] = 0;

	printf("\n/%s\\\n", border);
	printf(" %s", s);
	printf("\\%s/\n", border);
	free(border);
	displaycursor();
}


/// --- CURSOR --- \\\

static void displaycursor(){
	int line0 = 3, column0 = 1;
	int x = cursorpos.x, y = cursorpos.y;
	if(x != -1 && y != -1){
		savecurspos();
		gotoxy(column0+x,line0+y);
		fflush(stdout);
		loadcurspos();
	}
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
