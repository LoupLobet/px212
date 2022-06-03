#include <stdlib.h>
#include <stdio.h>
#include "display.h"
#include "util.h"

char spacetochar(Space *s);

void display(Map* map)
{
  printf("\e[1;1H\e[2J");
  if (*map->comment)
    printf("--> %s <--\n\n", map->comment);
  else
    printf("\n\n");

  int maxx = map->size.x, maxy = map->size.y;
  char* ligne = emalloc(sizeof(char)*maxy+1); //emalloc met à 0
  for (int x = 0; x < maxx; x++) {
    for (int y = 0; y < maxy; y++) {
      ligne[y] = spacetochar(&map->grid[x][y]);
    }
    printf("%s\n", ligne);
    for (int i = 0; i < maxy+1; i++) ligne[i] = 0;
  }
  if (*map->author)
    printf("\nMap n°%u from %s\n", map->id, map->author);
  else
    printf("\nMap n°%u \n", map->id);

  free(ligne);
}

char spacetochar(Space *s){
  char c = '!';
  switch (s->type) {
    case WALL:
      c = '#';
      break;
    case FLOOR:
      c = ' ';
      break;
    case TARGET:
      c = '.';
      break;
  }
  switch (s->content) {
    case PLAYER:
      c = '@';
      break;
    case BOX:
      c = '$';
      break;
    case EMPTY:
      break;
  }
  if(c == '!') error("spacetochar : invalid input space");
  return c;
}
