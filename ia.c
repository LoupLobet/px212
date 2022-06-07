#include "sokoban.h"
#include <stdlib.h>
##include <math.h>


typedef struct  {
    int traversable;
    int traverse;
} Node;


static node **createnodemap(Map* map, ){
	// init
	node **nodes = emalloc(sizeof(node*)*map->sise.x);
	for (int i = 0; i < x; i++) {
		nodes[i] = emalloc(sizeof(node)*map->sise.y);

	// conversion map
	for (int x = 0; x < map->sise.x; x++) {
		for (int y = 0; y < map->sise.y; y++) {
			Space space = map->grid[x][y];
			nodes[x][y].traversable = (space.type != WALL) && (space.content != BOX);
			nodes[x][y].traverse = 0;
		}
	}
	return nodes;
}


void freenodemap(int x, int y, Node **nodemap){
  for (int i = 0; i < x; i++) {
    free(nodemap[i]);
  }
  free(g);
}


Pair addpair(Pair pa, Pair pb){
	return (Pair){pa.x+pb.x, pa.y+pb.y};
}

Pair subpair(Pair pa, Pair pb){
	return (Pair){pa.x-pb.x, pa.y-pb.y};
}

float norm(Pair pa){
	return pa.x + pa.y;
}

int dist(Pair pa, Pair pb){
	Pair v = subpair(pa, pb);
	return v.a + v.b
}

void invert(Pair *v1, Pair *v2){
	Pair v3 = &v1;
	Pair v3
}



int nextnodes(Pair posbegin, Pair posend, int level, Pair *nodetable){
	Pair *nodes = {(Pair){1,0}, (Pair){0,1}, (Pair){-1,0}, (Pair){0,-1}};
	int nodedist[4];
	for (int i = 0; i < 4; i++) {
		nodetable
	}

}


Stack *playerwaybetween(Map* map, Pair posa, Pair posb) {

	node **nodes = createnodemap(map);



  // lancer recursivement les bonnes

  // si toutes mauvaises return NULL
  Stack *stack = malloc();

	freenodemap(map->sise.x, map->sise.y, nodes);
  return stack;
}

Stack *cratewaybetween(Map* map, Pair posa, Pair posb) {




}
