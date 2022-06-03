#include <stdlib.h>
#include <stdio.h>
#include "display.h"
#include "util.h"

char spacetochar(Space *s);


void display(Map* map)
{
  int maxx = map->size.x, maxy = map->size.y;
  for (int x = 0; x < maxx; x++) {
    char* ligne = emalloc(sizeof(char)*maxx+1); //emalloc met à 0
    for (int y = 0; y < maxy; y++) {
      ligne[y] = spacetochar(&map->grid[x][y]);
    }
    printf("%s\n", ligne);
    free(ligne); //TODO opti
  }
}

char spacetochar(Space *s){
  char c = '!';
  switch (s->type) {
    case WALL:
      c = 219;
      break;
    case FLOOR:
      c = ' ';
      break;
    case TARGET:
      c = 250;
      break;
  }
  switch ((*s).content) {
    case PLAYER:
      c = 173;
      break;
    case BOX:
      c = '#';
      break;
    case EMPTY:
      break;
  }
  if(c == '!') error("CA MARCHE PAS!!!");  //TODO meilleur msg
  return c;
}
