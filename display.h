#include "sokoban.h"


void display(Map* map);
void displaystr(char *s);
void displaywarning(char *fmt, ...);
void setcursor(Map *map, Pair pos);
void movecursor(Map *map, Pair mvt);
Pair getcursor();
