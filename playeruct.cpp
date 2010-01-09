/*
 * A UTC monte carlo player
 *
 */

#include "player.h"
#include <cmath>


class UTCNode {
public:
	int visits;
	int score;
	int numchildren;
	UTCNode *children;
	Board board;
	
	UTCNode(){
		visits = 0;
		score = 0;
		numchildren = 0;
		children = NULL;
	}
	
	UTCNode(const Board & nboard){
		visits = 0;
		score = 0;
		numchildren = 0;
		children = NULL;
		board = nboard;
	}
	
	~UTCNode(){
		if(numchildren)
			delete[] children;
	}

	void addChildren(int num, Board *boards){
		numchildren = num;
		UTCNode * temp = new UTCNode[num];
		for(int i = 0; i < num; i++)
			temp[i] = UTCNode(boards[i]);

		if(children == NULL) //atomic CAS!
			children = temp;
		else
			delete[] temp;
	}

	inline double winrate(){
		return (double)score/visits;
	}
};

class PlayerUCT : public Player {
	int maxruns;
	int maxnodes;
	double explore;  //greater than one favours exploration, smaller than one favours exploitation
	
	int remainmoves;
	int remainnodes;
	
	int minvisitschildren; //number of visits to a node before expanding its children nodes
	int minvisitspriority; //give priority to this node if it has less than this many visits
public:

	PlayerUCT(int numruns, int numnodes = 1000000000, double exploration = 1.0, int mvchildren = 1, int mvpriority = 1){
		maxnodes = numnodes;
		maxruns = numruns;
		explore = exploration;
		minvisitschildren = mvchildren;
		minvisitspriority = mvpriority;
	}

	~PlayerUCT(){
	}

	void describe(){
		printf("class PlayerUCT - a UCT monte carlo player playing %i runs with %i nodes and %.2f explore\n", maxruns, maxnodes, explore);
	}

	void print_stats(unsigned int runtime){
		printf("Moves: %u, Time: %u ms, Moves/s: %u\n", maxruns, runtime, (unsigned int)(1000.0*maxruns/runtime));
		totalmoves += maxruns;
	}

	int walk_tree(UTCNode & node, int depth){
		int result, won = -1;

		node.visits++; //atomic INCR

		if(node.children){
		//choose a child and recurse
			int maxi = 0;
			if(node.visits < node.numchildren/5){
				maxi = rand() % node.numchildren;
			}else{
				double val, maxval = -1000000000;
				double logvisits = log(node.visits);
				UTCNode * child;

				for(int i = 0; i < node.numchildren; i++){
					child = & node.children[i];

					if(child->visits < minvisitspriority) // give priority to nodes that have few or no visits
						val = 10000 - child->visits*1000 + rand()%100;
					else
						val = child->winrate() + explore*sqrt(logvisits/(5*child->visits));

					if(maxval < val){
						maxval = val;
						maxi = i;
					}
				}
			}

			result = - walk_tree(node.children[maxi], depth+1);
		}else if(node.visits <= minvisitschildren || remainnodes <= 0){
		//do random game on this node
			won = rand_game(node.board);
			depths[depth]++;
		}else if((won = node.board.won()) >= 0){
			//already done
		}else{
		//create children
			Board children[(36-node.board.nummoves)*8];
			int numchildren = node.board.getchildren(children, (depth < 3), false);

			node.addChildren(numchildren, children);
			remainnodes -= numchildren; //atomic

		//recurse on a random child
			result = - walk_tree(node.children[rand() % numchildren], depth+1);
		}
		if(won >= 0)
			result = (won == 0 ? 0 : won == node.board.turn() ? -1 : 1);

		node.score += result+1; //atomic
		return result;
	}

	Board search_move(Board board, bool output){
		UTCNode root(board);

		remainnodes = maxnodes;

	//call a recursive function to find a node and do a random game from there
		for(remainmoves = maxruns; remainmoves > 0; remainmoves--)
			walk_tree(root, 0);

		if(output){
		//print all the first depth results
			for(int i = 0; i < root.numchildren; i++)
				printf("%.2f/%i ", root.children[i].winrate(), root.children[i].visits);
			printf("\n");

			printf("Nodes generated: %i/%i\n", maxnodes - remainnodes, maxnodes);
		}

	//return the best one
		int maxi = 0;
		for(int i = 1; i < root.numchildren; i++)
//			if(root.children[maxi].winrate() < root.children[i].winrate())
			if(root.children[maxi].visits < root.children[i].visits)
				maxi = i;

		return root.children[maxi].board;
	}
};

