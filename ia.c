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

static Stack *redopath(PQelement **queueC, Node *n);
static int walkable(Pair pos, Map* map);
static Node *createnode(Pair pos, int cost, int heuristic, Pair camefrom);

static void insert(PQelement **queue, Node *node);
static Node* take(PQelement **queue, Pair pos);
static Node* pull(PQelement **queue);
static int isempty(PQelement *queue);
static int isin(PQelement *queue, Pair nodepos);
static void empty(PQelement **queue);


/**
 * @brief Finds the path for the player between two positions (if it exist).
 * @param map the map on wich the player evolve
 * @param posa the start position
 * @param posb the end position
 * @return Stack* the stack of moves to do to to go from posa to posb, NULL if there is no path
 */
Stack *playerwayto(Map* map, Pair posb){
	Pair posa = map->player;
	const Pair moves[] = {(Pair){-1, 0}, (Pair){0, -1}, (Pair){0, 1}, (Pair){1, 0}};

	PQelement *openlist = NULL;
	PQelement *closedlist = NULL;

	// node debut
	Node *debut = createnode(posa, 0, 0, posa);
	insert(&openlist, debut);

	while (!isempty(openlist)) {
		Node* current = pull(&openlist);
		if (pairequal(current->pos, posb)) {												// done
			Stack *s = redopath(&closedlist, current);
			empty(&openlist); empty(&closedlist); //clean
			return s;
		}
		for (int i = 0; i < 4; i++) { 															// each neighbor
			Pair neighborpos = pairadd(moves[i], current->pos);				// each neighbor
				if (walkable(neighborpos, map))  												// each neighbor
				{
				int tentativecost = current->cost + 1;
				if (!isin(closedlist, neighborpos)){										// not in closedlist
					if (isin(openlist, neighborpos)) {										// in openlist
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


/**
 * @brief Checks if a space is walkable.
 * @param pos the space position
 * @param map the map on wich the player evolve
 * @return int 1 if walkable, 0 else
 */
static int walkable(Pair pos, Map* map) {
	Space s = map->grid[pos.x][pos.y];
	return s.type != WALL && s.content != BOX;
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

/**
 * @brief Checks whether or not a node (designated by its position) is in the queue.
 * @param queue the queue
 * @param nodepos position attribute of the node to check for
 * @return int 1 if is in 0 else
 */
static int isin(PQelement *queue, Pair nodepos){
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

/**
 * @brief Calculates the signed manhatan distance between to pair
 * @param posa end position
 * @param posa start position
 * @return Pair the sum of pa and pb
 */
static int dist(Pair posa, Pair posb){
	return abs(posa.x - posb.x) + abs(posa.y - posb.y);
}
