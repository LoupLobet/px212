
#include "sokoban.h"
#include "display.h"
#include "move.h"
#include "ia.h"


typedef struct {
	Pair pos;
  int cost;
  int heuristic;
	Pair poscamefrom;
} Node;


typedef struct PQelement {
		Node *node;
    struct PQelement *next;
} PQelement;


static int pairequal(Pair pa, Pair pb);
static Pair pairadd(Pair pa, Pair pb);
static Pair pairsub(Pair pa, Pair pb);
static int dist(Pair posa, Pair posb);

static Stack *redopath(PQelement **queueO, PQelement **queueC, Node *n);
static int neighborvalid(Pair pos, Map* map);
static Node *createnode(Pair pos, int cost, int heuristic, Pair poscamefrom);

static void insert(PQelement **queue, Node *node);
static Node* take(PQelement **queue, Pair pos);
static Node* pull(PQelement **queue);
static int isempty(PQelement *queue);
static int isin(PQelement *queue, Pair nodepos);
static void empty(PQelement **queue);



//////////////// PAIR
int pairequal(Pair pa, Pair pb){
	return pa.x == pb.x && pa.y == pb.y;
}

Pair pairadd(Pair pa, Pair pb){
	return (Pair){pa.x + pb.x, pa.y + pb.y};
}

Pair pairsub(Pair pa, Pair pb){
	return (Pair){pa.x - pb.x, pa.y - pb.y};
}

int dist(Pair posa, Pair posb){
	return abs(posa.x - posb.x) + abs(posa.y - posb.y);
}
//////////////// PAIR


Stack *playerwaybetween(Map* map, Pair posa, Pair posb){
	static Pair moves[] = {(Pair){-1, 0}, (Pair){0, -1}, (Pair){0, 1}, (Pair){1, 0}};

	PQelement *openlist = NULL;
	PQelement *closedlist = NULL;

	// node debut
	Node *debut = createnode(posa, 0, 0, posa);
	insert(&openlist, debut);

	while (!isempty(openlist)) {
		Node* current = pull(&openlist);
		if (pairequal(current->pos, posb)) {												// done
			Stack *s = redopath(&openlist, &closedlist, current);
			empty(&openlist); empty(&closedlist); //clean
			return s;
		}
		for (int i = 0; i < 4; i++) { 															// each neighbor
			Pair neighborpos = pairadd(moves[i], current->pos);				// each neighbor
				if (neighborvalid(neighborpos, map))  									// each neighbor
				{
				int tentativecost = current->cost + 1;
				if (!isin(closedlist, neighborpos)){										// not in closedlist
					if (isin(openlist, neighborpos)) {										// in openlist
						Node *neighbornode = take(&openlist, neighborpos);
						if (tentativecost < neighbornode->cost){						// if.. modify it
							neighbornode->cost = tentativecost;
							neighbornode->heuristic = tentativecost + dist(neighborpos, posb);
							neighbornode->poscamefrom = current->pos;
							insert(&openlist, neighbornode);									// put it back
						}
					} else {																							// create it
						int h = tentativecost + dist(neighborpos, posb);
						Node *neighbornode = createnode(neighborpos, tentativecost, h, current->pos);
						insert(&openlist, neighbornode);
					}
				}
			}
		}
		insert(&closedlist, current);																// done with this one
	}
	empty(&openlist); empty(&closedlist); //clean
	return NULL;
}


int neighborvalid(Pair pos, Map* map) {
	Space s = map->grid[pos.x][pos.y];
	return s.type != WALL && s.content != BOX;
}

Node *createnode(Pair pos, int cost, int heuristic, Pair poscamefrom){
	Node *n = emalloc(sizeof(Node));
	n->pos = pos;
	n->cost = cost;
	n->heuristic = heuristic;
	n->poscamefrom = poscamefrom;
	return n;
}


Stack *redopath(PQelement **queueO, PQelement **queueC, Node *n){
	Stack	*s = NULL;
	while (!pairequal(n->pos, n->poscamefrom)) {
		Node *m = take(queueC, n->poscamefrom);
		if(m == NULL) m = take(queueO, n->poscamefrom);
		Pair move = pairsub(n->pos, m->pos);
		pushstack(&s, move, 0);
		free(n);
		n = m;
	}
	free(n);
	return s;
}


//////////////// PQ
void insert(PQelement **queue, Node *node){
	PQelement * element = emalloc(sizeof(PQelement));
	element->next = *queue;
	element->node = node;
	*queue = element;
}


Node* take(PQelement **queue, Pair pos){
	Node *n = NULL;
	if (isempty(*queue));																	//cas vide
	else if (pairequal((*queue)->node->pos, pos)){ 				//cas tête
		PQelement *e = *queue;
		n = (*queue)->node;
		*queue = (*queue)->next;
		free(e);
	}	else { 																						//cas autre
		PQelement *elementprec = *queue;
		PQelement *element = (*queue)->next;
		while (element != 0 && !pairequal(element->node->pos, pos)){
			elementprec = element;
			element = element->next;
		}
		elementprec->next = element->next;
		n = element->node;
		free(element);
	}
	return n;
}


Node* pull(PQelement **queue) {
	if(isempty(*queue)) return NULL;
	// find best element
	Pair minpos = (*queue)->node->pos;
	int minh = (*queue)->node->heuristic;
	PQelement *element = (*queue)->next;
	while (element != 0) {
		if(element->node->heuristic < minh) {
			minh = element->node->heuristic;
			minpos = element->node->pos;
		}
		element = element->next;
	}
	//take it (not optimised)
	return take(queue, minpos);
}


int isempty(PQelement *queue) {
	return queue == NULL;
}


int isin(PQelement *queue, Pair nodepos){
	if (isempty(queue)){
		return 0;
	} else {
		PQelement *element = queue;
		while (element != 0) {
			if (pairequal(element->node->pos, nodepos)) return 1;
			element = element->next;
		}
	}
	return 0;
}


void empty(PQelement **queue){
	while (!isempty(*queue)) free(pull(queue));
}
//////////////// PQ
