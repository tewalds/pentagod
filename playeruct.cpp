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
	
	UTCNode(Board nboard){
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
		children = new UTCNode[num];
		for(int i = 0; i < num; i++)
			children[i].board = boards[i];
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
	
	static const int minvisitschildren = 5; //number of visits to a node before expanding its children nodes
	static const int minvisitspriority = 3; //give priority to this node if it has less than this many visits
public:

	PlayerUCT(int numruns, int numnodes = 0, double exploration = 1.0){
		maxnodes = numnodes;
		maxruns = numruns;
		explore = exploration;
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

	int walk_tree(UTCNode & node){
		int result;

		node.visits++;

		if(node.children){
		//choose a child and recurse
			double val, maxval = -1000000000;
			double logvisits = log(node.visits);
			int maxi = 0;
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

			result = 1 - walk_tree(node.children[maxi]);
		}else if((result = node.board.won()) > 0 || node.board.nummoves == 36){
		//game is done at this node
			result = (result == 0 ? 0 : (result == node.board.turn() ? -1 : 1));
		}else if(node.visits < minvisitschildren || remainnodes <= 0){
		//do random game on this node
			result = rand_game(node.board);
		}else{
		//create children
			Board children[(36-node.board.nummoves)*8];
			int numchildren = node.board.getchildren(children, (node.board.nummoves < 20), false);

			node.addChildren(numchildren, children);
			remainnodes -= numchildren;

		//do random game on a random child. Could recurse, but this is faster
			int i = rand() % numchildren;

			result = 1 - rand_game(node.children[i].board);

			node.children[i].visits++;
			node.children[i].score += (result < 0) + (result <= 0);
		}
		
		node.score += (result > 0) + (result >= 0);
		return result;
	}

	Board search_move(Board board, bool output){
		UTCNode root(board);

		remainnodes = maxnodes;

	//call a recursive function to find a node and do a random game from there
		for(remainmoves = maxruns; remainmoves > 0; remainmoves--)
			walk_tree(root);

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

