#include <stdarg.h>
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

static void displaymap(Map* map, int leftmargin);
static void displayheader(Map* map);
static void updatecursor();
static char spacetochar(Space *s);

/**
 * @brief Display the game page (map, header, footer).
 * @param map map of the game
 * @param mvnb strokes number
 * @return void
 */
void display(Map* map)
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
	printf("\nYou made %d strokes\n", map->strokes);

	// print help
	// printf("\n - Press 'ESC' to quit \n - Press Arrow Keys to move\n - Press 'z' to undo\n - Press 's' to save\n - Press 'r' to restart\n - Press l nblevel and enter to choose a new level\n - Press 'n' to go to next level\n - Press 'p' to go to previous level\n - Press 'c' to pass in cursor mode and 'ESC' to exit\n");

	printf("\n\n");
}

/**
 * @brief Display a text at a fixed location : the bottom of the game page.
 * @param s string to display
 * @return void
 */
void displaystr(char *s) {
	printf("\e[1A\e[2K");
	printf("%s\n", s);
	updatecursor();
}

/**
 * @brief Display a warning at a fixed location : the bottom of the game page.
 * @param s warning text to display
 * @return void
 */
void displaywarning(char *fmt, ...) {
	va_list ap;

	printf("\e[1A\e[2K");
	va_start(ap, fmt);
	(void)printf("\e[31;47mWARNING : ");
	(void)vprintf(fmt, ap);
	va_end(ap);
	printf("\e[0m\n");
	updatecursor();
}

/**
 * @brief Display the map left aligned with a left margin.
 * @param map map to display
 * @param leftmargin left margin, 0: no margin
 * @return void
 */
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

/**
 * @brief Display the header and compute the data necessary to center the map and the header.
 * @param map map displayed after the header
 * @return void
 */
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


/// --- CURSOR --- ///
/**
 * @brief update the cursor according to the global position 'cursorpos'.
 * @param void
 * @return void
 */
static void updatecursor(void){
	int column0 = cursor0.x, line0 = cursor0.y;
	int x = cursorpos.x, y = cursorpos.y;
	if(x != -1 && y != -1){
		savecurspos();
		gotoxy(column0+x+1,line0+y+1);
		fflush(stdout);
		loadcurspos();
	} else fflush(stdout);
}

/// --- CURSOR --- ///
/**
 * @brief convert a Space to the char to be displayed.
 * @param s Space to convert
 * @return char char representing the Space
 */
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

/**
 * @brief set cursor position ((Pair){-1, -1} to hide it), while forcing it in the map boundaries.
 * @param map the map in wich the cursor evolves
 * @param pos the wished position of the cursor
 * @return void
 */
void setcursor(Map *map, Pair pos){
	int px = pos.x, py = pos.y;
	Pair ms = map->size;
	if(px == -1 && py == -1) cursorpos = pos;
	else cursorpos = (Pair){MIN(MAX(pos.x, 0), ms.x-1), MIN(MAX(pos.y, 0), ms.y-1)};
	updatecursor();
}

/**
 * @brief move cursor, while forcing it in the map boundaries.
 * @param map the map in wich the cursor evolves
 * @param pos the wished move to apply to the cursor
 * @return void
 */
void movecursor(Map *map, Pair mvt){
	setcursor(map, (Pair){cursorpos.x + mvt.x, cursorpos.y + mvt.y});
}

/**
 * @brief give the cursor position ((Pair){-1, -1} if hidden).
 * @param void
 * @return Pair{x,y} cursor position on the grid ((Pair){-1, -1} if hidden)
 */
Pair getcursor(void) {
	if (cursorpos.x ==-1 && cursorpos.y == -1) return (Pair){-1, -1};
	return (Pair){cursorpos.x - cursor0.x, cursorpos.y - cursor0.y};
}
