#include <stdlib.h>
#include <stdio.h>

#include "sokoban.h"
#include "display.h"
#include "move.h"
#include "util.h"
#include "ia.h"


typedef struct {
	Pair pos;
  int cost;
  int heuristic;
	Pair camefrom;
} Node;


typedef struct PQelement {
		Node *node;
    struct PQelement *next;
} PQelement;


static int pairequal(Pair pa, Pair pb);
static Pair pairadd(Pair pa, Pair pb);
static Pair pairsub(Pair pa, Pair pb);
static int dist(Pair posa, Pair posb);
static Pair pairopposite(Pair p);


static Stack *redopath(PQelement **queueC, Node *n);
static int walkable(Pair pos, Map* map);
static Node *createnode(Pair pos, int cost, int heuristic, Pair camefrom);

static void insert(PQelement **queue, Node *node);
static Node* take(PQelement **queue, Pair pos);
static Node* pull(PQelement **queue);
static int isempty(PQelement *queue);
static int isin(PQelement *queue, Pair nodepos);
static void empty(PQelement **queue);

static int validneighbourman(Node *current, Pair move, Map* map);
static int validneighbourbox(Node *current, Pair move, Map* map);
static int isinmgeneral(PQelement *queue, Pair nodepos, Pair prevpos, int (*elemequal)(PQelement*, Pair, Pair));
int elemequalman(PQelement *e, Pair nodepos, Pair prevpos);
int elemequalbox(PQelement *e, Pair nodepos, Pair prevpos);
static int isinman(PQelement *queue, Pair nodepos, Pair prevpos);
static int isinbox(PQelement *queue, Pair nodepos, Pair prevpos);

static Stack *waybetween(Map* map, Pair posa, Pair posb, int (*validneighbour)(Node*, Pair, Map*), int (*isin)(PQelement*, Pair, Pair));
static Stack *playerwaybetween(Map* map, Pair posa, Pair posb);
static Stack *concatstack(Stack *botom, Stack *top);

/**
 * @brief Finds the path for the player between two positions (if it exist).
 * @param map the map on wich the player evolve
 * @param posa the start position
 * @param posb the end position
 * @return Stack* the stack of moves to do to to go from posa to posb, NULL if there is no path
 */
Stack *playerwayto(Map* map, Pair pos){
	return playerwaybetween(map, map->player, pos);
}

static Stack *playerwaybetween(Map* map, Pair posa, Pair posb){
	return waybetween(map, posa, posb, &validneighbourman, &isinman);
}

Stack *boxwaybetween(Map* map, Pair posa, Pair posb){
	Stack *sbox = waybetween(map, posa, posb, &validneighbourbox, &isinbox);
	Stack *splayer = NULL;
	Pair posplayer = map->player;
	Pair posbox = posa;
	while(sbox != NULL){
		Stack *poped;	popstack(&sbox, poped);
		Pair boxmove = poped->move;
		Pair postomovebox = pairadd(pairopposite(boxmove), posbox);
		Stack *playertobox = playerwaybetween(map, posplayer, postomovebox);
		splayer = concatstack(splayer, playertobox);
		pushstack(&splayer, boxmove, 1);
		posplayer = pairadd(postomovebox, boxmove);
		posbox = pairadd(posbox, boxmove);
	}
	return splayer;
}

// stack related
static Stack *waybetween(Map* map, Pair posa, Pair posb, int (*validneighbour)(Node*, Pair, Map*), int (*isin)(PQelement*, Pair, Pair)){
	static Pair moves[] = {(Pair){-1, 0}, (Pair){0, -1}, (Pair){0, 1}, (Pair){1, 0}};

	PQelement *openlist = NULL;
	PQelement *closedlist = NULL;

	// node debut
	Node *debut = createnode(posa, 0, 0, posa);
	insert(&openlist, debut);

	while (!isempty(openlist)) {
		Node* current = pull(&openlist);
		if (pairequal(current->pos, posb)) {												// done
			Stack *s = redopath(&closedlist, current);			// TODO LA CHANGER POUR BOX
			empty(&openlist); empty(&closedlist); //clean
			return s;
		}
		for (int i = 0; i < 4; i++) { 															// each neighbor
				if ((*validneighbour)(current, moves[i], map)) 					// each neighbor
				{
				Pair neighborpos = pairadd(moves[i], current->pos);
				int tentativecost = current->cost + 1;
				if (!(*isin)(closedlist, neighborpos, current->pos)) {	// not in closedlist
					if ((*isin)(openlist, neighborpos, current->pos)) {		// in openlist
						Node *neighbornode = take(&openlist, neighborpos);
						if (tentativecost < neighbornode->cost){						// if.. modify it
							neighbornode->cost = tentativecost;
							neighbornode->heuristic = tentativecost + dist(neighborpos, posb);
							neighbornode->camefrom = current->pos;
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

// TODO oxy
static Stack *concatstack(Stack *botom, Stack *top){
	// empty top case
	if (top == 0) return botom;
	Stack *s = top;
	while (s->prev != 0) s = s->prev;
	s->prev = botom;
	return top;
}


// bonhomme
static int validneighbourman(Node *current, Pair move, Map* map) {
	Pair pos = pairadd(move, current->pos);
	Space s = map->grid[pos.x][pos.y];
	return s.type != WALL && s.content != BOX;
}

// caisse
static int validneighbourbox(Node *current, Pair move, Map* map) {
	// case have the space to move?
	Pair pos = pairadd(move, current->pos);
	Space s = map->grid[pos.x][pos.y];
	int spacefree = s.type != WALL && s.content != BOX;

	// player have can move it?
	Pair unmove = pairopposite(move);
	Pair playertargetpos = pairadd(unmove, current->pos);
	Pair playercurrentpos = current->camefrom;
	Stack *path = playerwaybetween(map, playercurrentpos, playertargetpos);
	if(path == NULL) return 0;
	else return spacefree;
}

/**
 * @brief Creates a node.
 * @param pos node's position
 * @param cost node's cost
 * @param heuristic node's heuristic
 * @param camefrom node's parent positin
 * @return Node* the created node
 */
static Node *createnode(Pair pos, int cost, int heuristic, Pair camefrom){
	Node *n = emalloc(sizeof(Node));
	n->pos = pos;
	n->cost = cost;
	n->heuristic = heuristic;
	n->camefrom = camefrom;
	return n;
}

// TODO finish

/**
 * @brief Creates the stack from the result of the A* algorithm by traveling the path backward.
 * @param pqueueO os node's position
 * @param queueC closed
 * @param heuristic node's heuristic
 * @param camefrom node's parent positin
 * @return Node* the created node
 */
static Stack *redopath(PQelement **queueC, Node *n){
	Stack	*s = NULL;
	while (!pairequal(n->pos, n->camefrom)) {
		Node *m = take(queueC, n->camefrom);
		Pair move = pairsub(n->pos, m->pos);
		pushstack(&s, move, 0);
		free(n);
		n = m;
	}
	free(n);
	return s;
}

/**
 * @brief Inserts an element in the priority queue.
 * @param queue head address of the queue
 * @param node node to insert ine the priority queue
 * @return void
 */
static void insert(PQelement **queue, Node *node){
	PQelement * element = emalloc(sizeof(PQelement));
	element->next = *queue;
	element->node = node;
	*queue = element;
}

/**
 * @brief Takes an element out of the priority queue, designated by its position .
 * @param queue head address of the queue
 * @param pos position attribute of the node to take
 * @return Node* the node taken from the queue, NULL if there is none
 */
static Node* take(PQelement **queue, Pair pos){
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

/**
 * @brief Pulls out of the priority queue the element with the smallest heuristic.
 * @param queue head address of the queue
 * @return Node* the node pulled from the queue, NULL if there is none
 */
static Node* pull(PQelement **queue) {
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

/**
 * @brief Checks whether or not the queue is empty.
 * @param queue the queue
 * @return int 1 if is empty 0 else
 */
static int isempty(PQelement *queue) {
	return queue == NULL;
}

//TODO doxy
/**
 * @brief Checks whether or not a node (designated by its position) is in the queue.
 * @param queue the queue
 * @param nodepos position attribute of the node to check for
 * @return int 1 if is in 0 else
 */
static int isinmgeneral(PQelement *queue, Pair nodepos, Pair prevpos, int (*elemequal)(PQelement*, Pair, Pair)){
	if (isempty(queue)){
		return 0;
	} else {
		PQelement *element = queue;
		while (element != 0) {
			if ((*elemequal)(element, nodepos, prevpos)) return 1;
			element = element->next;
		}
	}
	return 0;
}

int elemequalman(PQelement *e, Pair nodepos, Pair prevpos) {
	return pairequal(e->node->pos, nodepos);
}

int elemequalbox(PQelement *e, Pair nodepos, Pair prevpos) {
	return (pairequal(e->node->pos, nodepos) &&
					pairequal(e->node->camefrom, prevpos));
}

static int isinman(PQelement *queue, Pair nodepos, Pair prevpos){
	return isinmgeneral(queue, nodepos, prevpos, &elemequalman);
}

static int isinbox(PQelement *queue, Pair nodepos, Pair prevpos){
	return isinmgeneral(queue, nodepos, prevpos, &elemequalbox);
}

/**
 * @brief Emptys the queue and free the contained nodes
 * @param queue the queue
 * @return void
 */
static void empty(PQelement **queue){
	while (!isempty(*queue)) free(pull(queue));
}


/**
 * @brief Chechs whether or not two Pairs are equal
 * @param pa first pair to compare
 * @param pb second pair to compare
 * @return int 1 if equality, 0 else
 */
static int pairequal(Pair pa, Pair pb){
	return pa.x == pb.x && pa.y == pb.y;
}

/**
 * @brief Adds two Pairs
 * @param pa first pair to add
 * @param pb second pair to add
 * @return Pair the sum of pa and pb
 */
static Pair pairadd(Pair pa, Pair pb){
	return (Pair){pa.x + pb.x, pa.y + pb.y};
}

/**
 * @brief Substracts two Pairs
 * @param pa first pair
 * @param pb second pair to substract from the first one
 * @return Pair the substraction pa - pb
 */
static Pair pairsub(Pair pa, Pair pb){
	return (Pair){pa.x - pb.x, pa.y - pb.y};
}

//TODO doxy
static Pair pairopposite(Pair p){
	return (Pair){-p.x, -p.y};
}

/**
 * @brief Calculates the signed manhatan distance between to pair
 * @param posa end position
 * @param posa start position
 * @return Pair the sum of pa and pb
 */
static int dist(Pair posa, Pair posb){
	return abs(posa.x - posb.x) + abs(posa.y - posb.y);
}
