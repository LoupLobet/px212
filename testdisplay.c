#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "sokoban.h"
#include "display.h"


Space **creategrid(int x, int y){

  Space **returnedgrid = emalloc(sizeof(Space*)*y);
  for (int i = 0; i < y; i++) {
    returnedgrid[i] = emalloc(sizeof(Space)*x);
  }

  Space vide = {.type=FLOOR, .content=EMPTY};

  for (int i = 0; i < x; i++) {
    for (int j = 0; j < y; j++) {
      returnedgrid[i][j] = vide;
    }
  }
  returnedgrid[1][1] = (Space){.type=FLOOR, .content=BOX};
  return returnedgrid;
}

void free_grid(int x, int y, Space **g){
  for (int i = 0; i < y; i++) {
    free(g[i]);
  }
  free(g);
}


int main(){
  int x = 9, y = 9;
  Map m1 = {
    .size = (Pair){x, y},
    .player = (Pair){1, 1},
    .cursor = (Pair){-1, -1},
    .grid = creategrid(x, y),
    .comment = "hahahaha",
    .id = 4,
    .author = "Xavier dPdL"
  };

  display(&m1);
  free_grid(x, y, m1.grid);
}
