#include "sokoban.h"

void display(Map *, int);
void displaystr(char *);
void displaywarning(char *, ...);

void setcursor(Map *, Pair);
void movecursor(Map *, Pair);
Pair getcursor();
