#include <stdlib.h>
#include <stdio.h>
#include "display.h"
#include "util.h"

static char spacetochar(Space *s);

void display(Map* map)
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
  char* ligne = emalloc(sizeof(char)*maxx+1); //emalloc met à 0
  for (int y = 0; y < maxy; y++) {
    for (int x = 0; x < maxx; x++) {
      ligne[x] = spacetochar(&map->grid[x][y]);
    }
    //display player and cursor
    if (map->cursor.y == y)
      ligne[map->cursor.x] = 'O';
    if (map->player.y == y)
      ligne[map->player.x] = '@'; //TODO faire un choix entre map->player et map->grid[][].content
    printf("%s\n", ligne);
    for (int i = 0; i < maxx+1; i++) ligne[i] = 0;
  }
  // print author if exist and map id
  if (map->author && *map->author)
    printf("\nMap n°%u from %s\n", map->id, map->author);
  else
    printf("\nMap n°%u \n", map->id);

  free(ligne);
}

static char spacetochar(Space *s){
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
    case PLAYER:  //TODO faire un choix
      c = '@';
      break;
    case BOX:
      c = '$';
      break;
    case EMPTY:
      break;
  }
  // if(c == '!') error("spacetochar : invalid input space");  TODO regler ce pb
  return c;
}
